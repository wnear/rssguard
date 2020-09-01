// For license of this file, see <project-root-folder>/LICENSE.md.

#include "gui/webbrowser.h"

#include "gui/discoverfeedsbutton.h"
#include "gui/locationlineedit.h"
#include "gui/messagebox.h"
#include "gui/searchtextwidget.h"
#include "gui/webviewer.h"
#include "miscellaneous/application.h"
#include "miscellaneous/databasequeries.h"
#include "network-web/networkfactory.h"
#include "network-web/webfactory.h"
#include "services/abstract/serviceroot.h"

#include <QKeyEvent>
#include <QScrollBar>
#include <QToolBar>
#include <QToolTip>
#include <QWebEngineSettings>
#include <QWidgetAction>

WebBrowser::WebBrowser(QWidget *parent) : TabContent(parent),
    m_layout(new QVBoxLayout(this)),
    m_toolBar(new QToolBar(tr("Navigation panel"), this)),
    m_webView(new WebViewer(this)),
    m_searchWidget(new SearchTextWidget(this)),
    m_txtLocation(new LocationLineEdit(this)),
    m_btnDiscoverFeeds(new DiscoverFeedsButton(this)),
    m_actionBack(m_webView->pageAction(QWebEnginePage::Back)),
    m_actionForward(m_webView->pageAction(QWebEnginePage::Forward)),
    m_actionReload(m_webView->pageAction(QWebEnginePage::Reload)),
    m_actionStop(m_webView->pageAction(QWebEnginePage::Stop))
{
    // Initialize the components and layout.
    initializeLayout();
    setFocusProxy(m_txtLocation);
    setTabOrder(m_txtLocation, m_toolBar);
    setTabOrder(m_toolBar, m_webView);
    createConnections();
    reloadFontSettings();
}

void WebBrowser::createConnections()
{
    installEventFilter(this);

    connect(m_searchWidget, &SearchTextWidget::cancelSearch, this, [this]() {
        m_webView->findText(QString());
    });
    connect(m_searchWidget, &SearchTextWidget::searchForText, this, [this](const QString & text,
    bool backwards) {
        if (backwards) {
            m_webView->findText(text, QWebEnginePage::FindBackward);
        } else {
            m_webView->findText(text);
        }

        m_searchWidget->setFocus();
    });

    connect(m_webView, &WebViewer::messageStatusChangeRequested, this,
            &WebBrowser::receiveMessageStatusChangeRequest);
    connect(m_txtLocation, &LocationLineEdit::submitted,
            this, static_cast<void (WebBrowser::*)(const QString &)>(&WebBrowser::loadUrl));
    connect(m_webView, &WebViewer::urlChanged, this, &WebBrowser::updateUrl);

    // Change location textbox status according to webpage status.
    connect(m_webView, &WebViewer::loadStarted, this, &WebBrowser::onLoadingStarted);
    connect(m_webView, &WebViewer::loadProgress, this, &WebBrowser::onLoadingProgress);
    connect(m_webView, &WebViewer::loadFinished, this, &WebBrowser::onLoadingFinished);

    // Forward title/icon changes.
    connect(m_webView, &WebViewer::titleChanged, this, &WebBrowser::onTitleChanged);
    connect(m_webView, &WebViewer::iconChanged, this, &WebBrowser::onIconChanged);

    connect(m_webView->page(), &WebPage::windowCloseRequested, this, &WebBrowser::closeRequested);
}

void WebBrowser::updateUrl(const QUrl &url)
{
    m_txtLocation->setText(url.toString());

    //setNavigationBarVisible(url_string != INTERNAL_URL_EMPTY && url_string != INTERNAL_URL_NEWSPAPER);
}

void WebBrowser::loadUrl(const QUrl &url)
{
    if (url.isValid()) {
        m_webView->load(url);
    }
}

WebBrowser::~WebBrowser()
{
    // Delete members. Do not use scoped pointers here.
    delete m_layout;
}

void WebBrowser::reloadFontSettings()
{
    QFont fon;

    fon.fromString(qApp->settings()->value(GROUP(Messages),
                                           SETTING(Messages::PreviewerFontStandard)).toString());
    QWebEngineSettings::globalSettings()->setFontFamily(QWebEngineSettings::StandardFont, fon.family());
    QWebEngineSettings::globalSettings()->setFontSize(QWebEngineSettings::DefaultFontSize,
            fon.pointSize());
}

void WebBrowser::increaseZoom()
{
    m_webView->increaseWebPageZoom();
}

void WebBrowser::decreaseZoom()
{
    m_webView->decreaseWebPageZoom();
}

void WebBrowser::resetZoom()
{
    m_webView->resetWebPageZoom();
}

void WebBrowser::clear()
{
    m_webView->clear();
    m_messages.clear();
    hide();
}

void WebBrowser::loadUrl(const QString &url)
{
    return loadUrl(QUrl::fromUserInput(url));
}

void WebBrowser::loadMessages(const QList<Message> &messages, RootItem *root)
{
    m_messages = messages;
    m_root = root;

    if (!m_root.isNull()) {
        m_searchWidget->hide();
        m_webView->loadMessages(messages, root);
        show();
    }
}

void WebBrowser::loadMessage(const Message &message, RootItem *root)
{
    loadMessages(QList<Message>() << message, root);
}

bool WebBrowser::eventFilter(QObject *watched, QEvent *event)
{
    Q_UNUSED(watched)

    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *key_event = static_cast<QKeyEvent *>(event);

        if (key_event->matches(QKeySequence::StandardKey::Find)) {
            m_searchWidget->clear();
            m_searchWidget->show();
            m_searchWidget->setFocus();
            return true;
        }
    }

    return false;
}

void WebBrowser::receiveMessageStatusChangeRequest(int message_id,
        WebPage::MessageStatusChange change)
{
    switch (change) {
        case WebPage::MessageStatusChange::MarkRead:
            markMessageAsRead(message_id, true);
            break;

        case WebPage::MessageStatusChange::MarkUnread:
            markMessageAsRead(message_id, false);
            break;

        case WebPage::MessageStatusChange::MarkStarred:
            switchMessageImportance(message_id, true);
            break;

        case WebPage::MessageStatusChange::MarkUnstarred:
            switchMessageImportance(message_id, false);
            break;

        default:
            break;
    }
}

void WebBrowser::onTitleChanged(const QString &new_title)
{
    if (new_title.isEmpty()) {
        //: Webbrowser tab title when no title is available.
        emit titleChanged(m_index, tr("No title"));
    } else {
        emit titleChanged(m_index, new_title);
    }
}

void WebBrowser::onIconChanged(const QIcon &icon)
{
    emit iconChanged(m_index, icon);
}

void WebBrowser::initializeLayout()
{
    m_toolBar->setFloatable(false);
    m_toolBar->setMovable(false);
    m_toolBar->setAllowedAreas(Qt::TopToolBarArea);

    // Modify action texts.
    m_actionBack->setText(tr("Back"));
    m_actionBack->setToolTip(tr("Go back."));
    m_actionForward->setText(tr("Forward"));
    m_actionForward->setToolTip(tr("Go forward."));
    m_actionReload->setText(tr("Reload"));
    m_actionReload->setToolTip(tr("Reload current web page."));
    m_actionStop->setText(tr("Stop"));
    m_actionStop->setToolTip(tr("Stop web page loading."));
    QWidgetAction *act_discover = new QWidgetAction(this);

    act_discover->setDefaultWidget(m_btnDiscoverFeeds);

    // Add needed actions into toolbar.
    m_toolBar->addAction(m_actionBack);
    m_toolBar->addAction(m_actionForward);
    m_toolBar->addAction(m_actionReload);
    m_toolBar->addAction(m_actionStop);
    m_toolBar->addAction(act_discover);
    m_toolBar->addWidget(m_txtLocation);
    m_loadingProgress = new QProgressBar(this);
    m_loadingProgress->setFixedHeight(5);
    m_loadingProgress->setMinimum(0);
    m_loadingProgress->setTextVisible(false);
    m_loadingProgress->setMaximum(100);
    m_loadingProgress->setAttribute(Qt::WA_TranslucentBackground);

    // Setup layout.
    m_layout->addWidget(m_toolBar);
    m_layout->addWidget(m_webView);
    m_layout->addWidget(m_loadingProgress);
    m_layout->addWidget(m_searchWidget);
    m_layout->setMargin(0);
    m_layout->setSpacing(0);

    m_searchWidget->hide();
}

void WebBrowser::onLoadingStarted()
{
    m_btnDiscoverFeeds->clearFeedAddresses();
    m_loadingProgress->show();
}

void WebBrowser::onLoadingProgress(int progress)
{
    m_loadingProgress->setValue(progress);
}

void WebBrowser::onLoadingFinished(bool success)
{
    if (success) {
        // Let's check if there are any feeds defined on the web and eventually
        // display "Add feeds" button.
        m_webView->page()->toHtml([this](const QString & result) {
            this->m_btnDiscoverFeeds->setFeedAddresses(NetworkFactory::extractFeedLinksFromHtmlPage(
                        m_webView->url(), result));
        });
    } else {
        m_btnDiscoverFeeds->clearFeedAddresses();
    }

    m_loadingProgress->hide();
    m_loadingProgress->setValue(0);
}

void WebBrowser::markMessageAsRead(int id, bool read)
{
    if (!m_root.isNull()) {
        Message *msg = findMessage(id);

        if (msg != nullptr && m_root->getParentServiceRoot()->onBeforeSetMessagesRead(m_root.data(),
                QList<Message>() << *msg,
                read
                ? RootItem::ReadStatus::Read
                : RootItem::ReadStatus::Unread)) {
            DatabaseQueries::markMessagesReadUnread(qApp->database()->connection(objectName()),
                                                    QStringList() << QString::number(msg->m_id),
                                                    read ? RootItem::ReadStatus::Read : RootItem::ReadStatus::Unread);
            m_root->getParentServiceRoot()->onAfterSetMessagesRead(m_root.data(),
                    QList<Message>() << *msg,
                    read ? RootItem::ReadStatus::Read : RootItem::ReadStatus::Unread);
            emit markMessageRead(msg->m_id, read ? RootItem::ReadStatus::Read : RootItem::ReadStatus::Unread);

            msg->m_isRead = read;
        }
    }
}

void WebBrowser::switchMessageImportance(int id, bool checked)
{
    if (!m_root.isNull()) {
        Message *msg = findMessage(id);

        if (msg != nullptr &&
                m_root->getParentServiceRoot()->onBeforeSwitchMessageImportance(m_root.data(),
                        QList<ImportanceChange>()
                        << ImportanceChange(*msg,
                                            msg->m_isImportant
                                            ? RootItem::Importance::NotImportant
                                            : RootItem::Importance::Important))) {
            DatabaseQueries::switchMessagesImportance(qApp->database()->connection(objectName()),
                    QStringList() << QString::number(msg->m_id));
            m_root->getParentServiceRoot()->onAfterSwitchMessageImportance(m_root.data(),
                    QList<ImportanceChange>()
                    << ImportanceChange(*msg,
                                        msg->m_isImportant ?
                                        RootItem::Importance::NotImportant :
                                        RootItem::Importance::Important));
            emit markMessageImportant(msg->m_id, msg->m_isImportant
                                      ? RootItem::Importance::NotImportant
                                      : RootItem::Importance::Important);

            msg->m_isImportant = checked;
        }
    }
}

Message *WebBrowser::findMessage(int id)
{
    for (int i = 0; i < m_messages.size(); i++) {
        if (m_messages.at(i).m_id == id) {
            return &m_messages[i];
        }
    }

    return nullptr;
}
