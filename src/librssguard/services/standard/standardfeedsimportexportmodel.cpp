// For license of this file, see <project-root-folder>/LICENSE.md.

#include "services/standard/standardfeedsimportexportmodel.h"

#include "definitions/definitions.h"
#include "miscellaneous/application.h"
#include "miscellaneous/iconfactory.h"
#include "services/standard/standardcategory.h"
#include "services/standard/standardfeed.h"
#include "services/standard/standardserviceroot.h"

#include <QDomAttr>
#include <QDomDocument>
#include <QDomElement>
#include <QLocale>
#include <QStack>

FeedsImportExportModel::FeedsImportExportModel(QObject *parent)
    : AccountCheckModel(parent), m_mode(Mode::Import) {}

FeedsImportExportModel::~FeedsImportExportModel()
{
    if (m_rootItem != nullptr && m_mode == Mode::Import) {
        // Delete all model items, but only if we are in import mode. Export mode shares
        // root item with main feed model, thus cannot be deleted from memory now.
        delete m_rootItem;
    }
}

bool FeedsImportExportModel::exportToOMPL20(QByteArray &result)
{
    QDomDocument opml_document;
    QDomProcessingInstruction xml_declaration = opml_document.createProcessingInstruction(QSL("xml"),
            QSL(
                "version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\""));

    opml_document.appendChild(xml_declaration);

    // Added OPML 2.0 metadata.
    opml_document.appendChild(opml_document.createElement(QSL("opml")));
    opml_document.documentElement().setAttribute(QSL("version"), QSL("2.0"));
    opml_document.documentElement().setAttribute("xmlns:rssguard", APP_URL);
    QDomElement elem_opml_head = opml_document.createElement(QSL("head"));
    QDomElement elem_opml_title = opml_document.createElement(QSL("title"));
    QDomText text_opml_title = opml_document.createTextNode(QString(APP_NAME));

    elem_opml_title.appendChild(text_opml_title);
    elem_opml_head.appendChild(elem_opml_title);
    QDomElement elem_opml_created = opml_document.createElement(QSL("dateCreated"));
    QDomText text_opml_created = opml_document.createTextNode(QLocale::c().toString(
                                     QDateTime::currentDateTimeUtc(),
                                     QSL("ddd, dd MMM yyyy hh:mm:ss")) + QL1S(" GMT"));

    elem_opml_created.appendChild(text_opml_created);
    elem_opml_head.appendChild(elem_opml_created);
    opml_document.documentElement().appendChild(elem_opml_head);
    QDomElement elem_opml_body = opml_document.createElement(QSL("body"));
    QStack<RootItem *> items_to_process;

    items_to_process.push(m_rootItem);
    QStack<QDomElement> elements_to_use;

    elements_to_use.push(elem_opml_body);

    // Process all unprocessed nodes.
    while (!items_to_process.isEmpty()) {
        QDomElement active_element = elements_to_use.pop();
        RootItem *active_item = items_to_process.pop();

        for (RootItem *child_item : active_item->childItems()) {
            if (!isItemChecked(child_item)) {
                continue;
            }

            switch (child_item->kind()) {
                case RootItem::Kind::Category: {
                    QDomElement outline_category = opml_document.createElement(QSL("outline"));

                    outline_category.setAttribute(QSL("text"), child_item->title());
                    outline_category.setAttribute(QSL("description"), child_item->description());
                    outline_category.setAttribute(QSL("rssguard:icon"),
                                                  QString(qApp->icons()->toByteArray(child_item->icon())));
                    active_element.appendChild(outline_category);
                    items_to_process.push(child_item);
                    elements_to_use.push(outline_category);
                    break;
                }

                case RootItem::Kind::Feed: {
                    auto *child_feed = dynamic_cast<StandardFeed *>(child_item);
                    QDomElement outline_feed = opml_document.createElement("outline");

                    outline_feed.setAttribute(QSL("type"), QSL("rss"));
                    outline_feed.setAttribute(QSL("text"), child_feed->title());
                    outline_feed.setAttribute(QSL("xmlUrl"), child_feed->url());
                    outline_feed.setAttribute(QSL("description"), child_feed->description());
                    outline_feed.setAttribute(QSL("encoding"), child_feed->encoding());
                    outline_feed.setAttribute(QSL("title"), child_feed->title());
                    outline_feed.setAttribute(QSL("rssguard:icon"),
                                              QString(qApp->icons()->toByteArray(child_feed->icon())));

                    switch (child_feed->type()) {
                        case StandardFeed::Type::Rss0X:
                        case StandardFeed::Type::Rss2X:
                            outline_feed.setAttribute(QSL("version"), QSL("RSS"));
                            break;

                        case StandardFeed::Type::Rdf:
                            outline_feed.setAttribute(QSL("version"), QSL("RSS1"));
                            break;

                        case StandardFeed::Type::Atom10:
                            outline_feed.setAttribute(QSL("version"), QSL("ATOM"));
                            break;

                        default:
                            break;
                    }

                    active_element.appendChild(outline_feed);
                    break;
                }

                default:
                    break;
            }
        }
    }

    opml_document.documentElement().appendChild(elem_opml_body);
    result = opml_document.toByteArray(2);
    return true;
}

void FeedsImportExportModel::importAsOPML20(const QByteArray &data, bool fetch_metadata_online)
{
    emit parsingStarted();
    emit layoutAboutToBeChanged();

    setRootItem(nullptr);
    emit layoutChanged();
    QDomDocument opml_document;

    if (!opml_document.setContent(data)) {
        emit parsingFinished(0, 0, true);
    }

    if (opml_document.documentElement().isNull()
            || opml_document.documentElement().tagName() != QSL("opml") ||
            opml_document.documentElement().elementsByTagName(QSL("body")).size() != 1) {
        // This really is not an OPML file.
        emit parsingFinished(0, 0, true);
    }

    int completed = 0, total = 0, succeded = 0, failed = 0;
    auto *root_item = new StandardServiceRoot();
    QStack<RootItem *> model_items;

    model_items.push(root_item);
    QStack<QDomElement> elements_to_process;

    elements_to_process.push(opml_document.documentElement().elementsByTagName(QSL("body")).at(
                                 0).toElement());

    while (!elements_to_process.isEmpty()) {
        RootItem *active_model_item = model_items.pop();
        QDomElement active_element = elements_to_process.pop();
        int current_count = active_element.childNodes().size();

        total += current_count;

        for (int i = 0; i < current_count; i++) {
            QDomNode child = active_element.childNodes().at(i);

            if (child.isElement()) {
                QDomElement child_element = child.toElement();

                // Now analyze if this element is category or feed.
                // NOTE: All feeds must include xmlUrl attribute and text attribute.
                if (child_element.attributes().contains(QSL("xmlUrl"))
                        && child.attributes().contains(QSL("text"))) {
                    // This is FEED.
                    // Add feed and end this iteration.
                    QString feed_url = child_element.attribute(QSL("xmlUrl"));

                    if (!feed_url.isEmpty()) {
                        QPair<StandardFeed *, QNetworkReply::NetworkError> guessed;

                        if (fetch_metadata_online &&
                                (guessed = StandardFeed::guessFeed(feed_url)).second == QNetworkReply::NoError) {
                            // We should obtain fresh metadata from online feed source.
                            guessed.first->setUrl(feed_url);
                            active_model_item->appendChild(guessed.first);
                            succeded++;
                        } else {
                            QString feed_title = child_element.attribute(QSL("text"));
                            QString feed_encoding = child_element.attribute(QSL("encoding"), DEFAULT_FEED_ENCODING);
                            QString feed_type = child_element.attribute(QSL("version"), DEFAULT_FEED_TYPE).toUpper();
                            QString feed_description = child_element.attribute(QSL("description"));
                            QIcon feed_icon = qApp->icons()->fromByteArray(child_element.attribute(
                                                  QSL("rssguard:icon")).toLocal8Bit());
                            auto *new_feed = new StandardFeed(active_model_item);

                            new_feed->setTitle(feed_title);
                            new_feed->setDescription(feed_description);
                            new_feed->setEncoding(feed_encoding);
                            new_feed->setUrl(feed_url);
                            new_feed->setCreationDate(QDateTime::currentDateTime());
                            new_feed->setIcon(feed_icon);

                            if (feed_type == QL1S("RSS1")) {
                                new_feed->setType(StandardFeed::Type::Rdf);
                            } else if (feed_type == QL1S("ATOM")) {
                                new_feed->setType(StandardFeed::Type::Atom10);
                            } else {
                                new_feed->setType(StandardFeed::Type::Rss2X);
                            }

                            active_model_item->appendChild(new_feed);

                            if (fetch_metadata_online && guessed.second != QNetworkReply::NoError) {
                                failed++;
                            } else {
                                succeded++;
                            }
                        }
                    }
                } else {
                    // This must be CATEGORY.
                    // Add category and continue.
                    QString category_title = child_element.attribute(QSL("text"));
                    QString category_description = child_element.attribute(QSL("description"));
                    QIcon category_icon = qApp->icons()->fromByteArray(child_element.attribute(
                                              QSL("rssguard:icon")).toLocal8Bit());

                    if (category_title.isEmpty()) {
                        qWarning("Given OMPL file provided category without valid text attribute. Using fallback name.");
                        category_title = child_element.attribute(QSL("title"));

                        if (category_title.isEmpty()) {
                            category_title = tr("Category ") + QString::number(
                                                 QDateTime::currentDateTime().toMSecsSinceEpoch());
                        }
                    }

                    auto *new_category = new StandardCategory(active_model_item);

                    new_category->setTitle(category_title);
                    new_category->setIcon(category_icon);
                    new_category->setCreationDate(QDateTime::currentDateTime());
                    new_category->setDescription(category_description);
                    active_model_item->appendChild(new_category);

                    // Children of this node must be processed later.
                    elements_to_process.push(child_element);
                    model_items.push(new_category);
                }

                emit parsingProgress(++completed, total);
            }
        }
    }

    // Now, XML is processed and we have result in form of pointer item structure.
    emit layoutAboutToBeChanged();

    setRootItem(root_item);
    emit layoutChanged();
    emit parsingFinished(failed, succeded, false);
}

bool FeedsImportExportModel::exportToTxtURLPerLine(QByteArray &result)
{
    for (const Feed *const feed : m_rootItem->getSubTreeFeeds()) {
        result += feed->url() + QL1S("\n");
    }

    return true;
}

void FeedsImportExportModel::importAsTxtURLPerLine(const QByteArray &data,
        bool fetch_metadata_online)
{
    emit parsingStarted();
    emit layoutAboutToBeChanged();

    setRootItem(nullptr);
    emit layoutChanged();
    int completed = 0, succeded = 0, failed = 0;
    auto *root_item = new StandardServiceRoot();
    QList<QByteArray> urls = data.split('\n');

    for (const QByteArray &url : urls) {
        if (!url.isEmpty()) {
            QPair<StandardFeed *, QNetworkReply::NetworkError> guessed;

            if (fetch_metadata_online &&
                    (guessed = StandardFeed::guessFeed(url)).second == QNetworkReply::NoError) {
                guessed.first->setUrl(url);
                root_item->appendChild(guessed.first);
                succeded++;
            } else {
                auto *feed = new StandardFeed();

                feed->setUrl(url);
                feed->setTitle(url);
                feed->setCreationDate(QDateTime::currentDateTime());
                feed->setIcon(qApp->icons()->fromTheme(QSL("application-rss+xml")));
                feed->setEncoding(DEFAULT_FEED_ENCODING);
                root_item->appendChild(feed);

                if (fetch_metadata_online && guessed.second != QNetworkReply::NoError) {
                    failed++;
                } else {
                    succeded++;
                }
            }

            qApp->processEvents();
        } else {
            qWarning("Detected empty URL when parsing input TXT [one URL per line] data.");
            failed++;
        }

        emit parsingProgress(++completed, urls.size());
    }

    // Now, XML is processed and we have result in form of pointer item structure.
    emit layoutAboutToBeChanged();

    setRootItem(root_item);
    emit layoutChanged();
    emit parsingFinished(failed, succeded, false);
}

FeedsImportExportModel::Mode FeedsImportExportModel::mode() const
{
    return m_mode;
}

void FeedsImportExportModel::setMode(const FeedsImportExportModel::Mode &mode)
{
    m_mode = mode;
}
