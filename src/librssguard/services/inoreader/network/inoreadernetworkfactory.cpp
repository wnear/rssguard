// For license of this file, see <project-root-folder>/LICENSE.md.

#include "services/inoreader/network/inoreadernetworkfactory.h"

#include "definitions/definitions.h"
#include "gui/dialogs/formmain.h"
#include "gui/tabwidget.h"
#include "miscellaneous/application.h"
#include "miscellaneous/databasequeries.h"
#include "network-web/networkfactory.h"
#include "network-web/oauth2service.h"
#include "network-web/silentnetworkaccessmanager.h"
#include "network-web/webfactory.h"
#include "services/abstract/category.h"
#include "services/inoreader/definitions.h"
#include "services/inoreader/inoreaderfeed.h"
#include "services/inoreader/inoreaderserviceroot.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QUrl>

InoreaderNetworkFactory::InoreaderNetworkFactory(QObject *parent) : QObject(parent),
    m_service(nullptr), m_username(QString()), m_batchSize(INOREADER_DEFAULT_BATCH_SIZE),
    m_oauth2(new OAuth2Service(INOREADER_OAUTH_AUTH_URL, INOREADER_OAUTH_TOKEN_URL,
                               INOREADER_OAUTH_CLI_ID, INOREADER_OAUTH_CLI_KEY, INOREADER_OAUTH_SCOPE, this))
{
    initializeOauth();
}

void InoreaderNetworkFactory::setService(InoreaderServiceRoot *service)
{
    m_service = service;
}

OAuth2Service *InoreaderNetworkFactory::oauth() const
{
    return m_oauth2;
}

QString InoreaderNetworkFactory::userName() const
{
    return m_username;
}

int InoreaderNetworkFactory::batchSize() const
{
    return m_batchSize;
}

void InoreaderNetworkFactory::setBatchSize(int batch_size)
{
    m_batchSize = batch_size;
}

void InoreaderNetworkFactory::initializeOauth()
{
    connect(m_oauth2, &OAuth2Service::tokensRetrieveError, this,
            &InoreaderNetworkFactory::onTokensError);
    connect(m_oauth2, &OAuth2Service::authFailed, this, &InoreaderNetworkFactory::onAuthFailed);
    connect(m_oauth2, &OAuth2Service::tokensReceived, this, [this](QString access_token,
    QString refresh_token, int expires_in) {
        Q_UNUSED(expires_in)
        Q_UNUSED(access_token)

        if (m_service != nullptr && !refresh_token.isEmpty()) {
            QSqlDatabase database = qApp->database()->connection(metaObject()->className());
            DatabaseQueries::storeNewInoreaderTokens(database, refresh_token, m_service->accountId());

            qApp->showGuiMessage(tr("Logged in successfully"),
                                 tr("Your login to Inoreader was authorized."),
                                 QSystemTrayIcon::MessageIcon::Information);
        }
    });
}

void InoreaderNetworkFactory::setUsername(const QString &username)
{
    m_username = username;
}

// NOTE: oauth: https://developers.google.com/oauthplayground/#step3&scopes=read%20write&auth_code=497815bc3362aba9ad60c5ae3e01811fe2da4bb5&refresh_token=bacb9c36f82ba92667282d6175bb857a091e7f0c&access_token_field=094f92bc7aedbd27fbebc3efc9172b258be8944a&url=https%3A%2F%2Fwww.inoreader.com%2Freader%2Fapi%2F0%2Fsubscription%2Flist&content_type=application%2Fjson&http_method=GET&useDefaultOauthCred=unchecked&oauthEndpointSelect=Custom&oauthAuthEndpointValue=https%3A%2F%2Fwww.inoreader.com%2Foauth2%2Fauth%3Fstate%3Dtest&oauthTokenEndpointValue=https%3A%2F%2Fwww.inoreader.com%2Foauth2%2Ftoken&oauthClientId=1000000595&expires_in=3599&oauthClientSecret=_6pYUZgtNLWwSaB9pC1YOz6p4zwu3haL&access_token_issue_date=1506198338&for_access_token=094f92bc7aedbd27fbebc3efc9172b258be8944a&includeCredentials=checked&accessTokenType=bearer&autoRefreshToken=unchecked&accessType=offline&prompt=consent&response_type=code

RootItem *InoreaderNetworkFactory::feedsCategories(bool obtain_icons)
{
    Downloader downloader;
    QEventLoop loop;
    QString bearer = m_oauth2->bearer().toLocal8Bit();

    if (bearer.isEmpty()) {
        return nullptr;
    }

    downloader.appendRawHeader(QString(HTTP_HEADERS_AUTHORIZATION).toLocal8Bit(), bearer.toLocal8Bit());

    // We need to quit event loop when the download finishes.
    connect(&downloader, &Downloader::completed, &loop, &QEventLoop::quit);
    downloader.downloadFile(INOREADER_API_LIST_LABELS, qApp->settings()->value(GROUP(Feeds),
                            SETTING(Feeds::UpdateTimeout)).toInt());
    loop.exec();

    if (downloader.lastOutputError() != QNetworkReply::NetworkError::NoError) {
        return nullptr;
    }

    QString category_data = downloader.lastOutputData();

    downloader.manipulateData(INOREADER_API_LIST_FEEDS, QNetworkAccessManager::Operation::GetOperation);
    loop.exec();

    if (downloader.lastOutputError() != QNetworkReply::NetworkError::NoError) {
        return nullptr;
    }

    QString feed_data = downloader.lastOutputData();

    return decodeFeedCategoriesData(category_data, feed_data, obtain_icons);
}

QList<Message> InoreaderNetworkFactory::messages(const QString &stream_id, Feed::Status &error)
{
    Downloader downloader;
    QEventLoop loop;
    QString target_url = INOREADER_API_FEED_CONTENTS;
    QString bearer = m_oauth2->bearer().toLocal8Bit();

    if (bearer.isEmpty()) {
        qCritical("Cannot download messages for '%s', bearer is empty.", qPrintable(stream_id));
        error = Feed::Status::AuthError;
        return QList<Message>();
    }

    target_url += QSL("/") + QUrl::toPercentEncoding(stream_id) + QString("?n=%1").arg(batchSize());
    downloader.appendRawHeader(QString(HTTP_HEADERS_AUTHORIZATION).toLocal8Bit(), bearer.toLocal8Bit());

    // We need to quit event loop when the download finishes.
    connect(&downloader, &Downloader::completed, &loop, &QEventLoop::quit);
    downloader.downloadFile(target_url, qApp->settings()->value(GROUP(Feeds),
                            SETTING(Feeds::UpdateTimeout)).toInt());
    loop.exec();

    if (downloader.lastOutputError() != QNetworkReply::NetworkError::NoError) {
        qCritical("Cannot download messages for '%s', network error: %d.", qPrintable(stream_id),
                  int(downloader.lastOutputError()));
        error = Feed::Status::NetworkError;
        return QList<Message>();
    } else {
        QString messages_data = downloader.lastOutputData();

        error = Feed::Status::Normal;
        return decodeMessages(messages_data, stream_id);
    }
}

void InoreaderNetworkFactory::markMessagesRead(RootItem::ReadStatus status,
        const QStringList &custom_ids, bool async)
{
    QString target_url = INOREADER_API_EDIT_TAG;

    if (status == RootItem::ReadStatus::Read) {
        target_url += QString("?a=user/-/") + INOREADER_STATE_READ + "&";
    } else {
        target_url += QString("?r=user/-/") + INOREADER_STATE_READ + "&";
    }

    QString bearer = m_oauth2->bearer().toLocal8Bit();

    if (bearer.isEmpty()) {
        return;
    }

    QList<QPair<QByteArray, QByteArray>> headers;

    headers.append(QPair<QByteArray, QByteArray>(QString(HTTP_HEADERS_AUTHORIZATION).toLocal8Bit(),
                   m_oauth2->bearer().toLocal8Bit()));

    QStringList trimmed_ids;
    QRegularExpression regex_short_id(QSL("[0-9a-zA-Z]+$"));

    for (const QString &id : custom_ids) {
        QString simplified_id = regex_short_id.match(id).captured();

        trimmed_ids.append(QString("i=") + simplified_id);
    }

    QStringList working_subset;
    int timeout = qApp->settings()->value(GROUP(Feeds), SETTING(Feeds::UpdateTimeout)).toInt();

    working_subset.reserve(trimmed_ids.size() > 200 ? 200 : trimmed_ids.size());

    // Now, we perform messages update in batches (max 200 messages per batch).
    while (!trimmed_ids.isEmpty()) {
        // We take 200 IDs.
        for (int i = 0; i < 200 && !trimmed_ids.isEmpty(); i++) {
            working_subset.append(trimmed_ids.takeFirst());
        }

        QString batch_final_url = target_url + working_subset.join(QL1C('&'));

        // We send this batch.
        if (async) {

            NetworkFactory::performAsyncNetworkOperation(batch_final_url,
                    timeout,
                    QByteArray(),
                    QNetworkAccessManager::Operation::GetOperation,
                    headers);
        } else {
            QByteArray output;

            NetworkFactory::performNetworkOperation(batch_final_url,
                                                    timeout,
                                                    QByteArray(),
                                                    output,
                                                    QNetworkAccessManager::Operation::GetOperation,
                                                    headers);
        }

        // Cleanup for next batch.
        working_subset.clear();
    }
}

void InoreaderNetworkFactory::markMessagesStarred(RootItem::Importance importance,
        const QStringList &custom_ids, bool async)
{
    QString target_url = INOREADER_API_EDIT_TAG;

    if (importance == RootItem::Importance::Important) {
        target_url += QString("?a=user/-/") + INOREADER_STATE_IMPORTANT + "&";
    } else {
        target_url += QString("?r=user/-/") + INOREADER_STATE_IMPORTANT + "&";
    }

    QString bearer = m_oauth2->bearer().toLocal8Bit();

    if (bearer.isEmpty()) {
        return;
    }

    QList<QPair<QByteArray, QByteArray>> headers;

    headers.append(QPair<QByteArray, QByteArray>(QString(HTTP_HEADERS_AUTHORIZATION).toLocal8Bit(),
                   m_oauth2->bearer().toLocal8Bit()));

    QStringList trimmed_ids;
    QRegularExpression regex_short_id(QSL("[0-9a-zA-Z]+$"));

    for (const QString &id : custom_ids) {
        QString simplified_id = regex_short_id.match(id).captured();

        trimmed_ids.append(QString("i=") + simplified_id);
    }

    QStringList working_subset;
    int timeout = qApp->settings()->value(GROUP(Feeds), SETTING(Feeds::UpdateTimeout)).toInt();

    working_subset.reserve(trimmed_ids.size() > 200 ? 200 : trimmed_ids.size());

    // Now, we perform messages update in batches (max 200 messages per batch).
    while (!trimmed_ids.isEmpty()) {
        // We take 200 IDs.
        for (int i = 0; i < 200 && !trimmed_ids.isEmpty(); i++) {
            working_subset.append(trimmed_ids.takeFirst());
        }

        QString batch_final_url = target_url + working_subset.join(QL1C('&'));

        // We send this batch.
        if (async) {
            NetworkFactory::performAsyncNetworkOperation(batch_final_url,
                    timeout,
                    QByteArray(),
                    QNetworkAccessManager::Operation::GetOperation,
                    headers);
        } else {
            QByteArray output;

            NetworkFactory::performNetworkOperation(batch_final_url,
                                                    timeout,
                                                    QByteArray(),
                                                    output,
                                                    QNetworkAccessManager::Operation::GetOperation,
                                                    headers);
        }

        // Cleanup for next batch.
        working_subset.clear();
    }
}

void InoreaderNetworkFactory::onTokensError(const QString &error, const QString &error_description)
{
    Q_UNUSED(error)

    qApp->showGuiMessage(tr("Inoreader: authentication error"),
                         tr("Click this to login again. Error is: '%1'").arg(error_description),
                         QSystemTrayIcon::Critical,
                         nullptr, false,
    [this]() {
        m_oauth2->setAccessToken(QString());
        m_oauth2->setRefreshToken(QString());
        m_oauth2->login();
    });
}

void InoreaderNetworkFactory::onAuthFailed()
{
    qApp->showGuiMessage(tr("Inoreader: authorization denied"),
                         tr("Click this to login again."),
                         QSystemTrayIcon::Critical,
                         nullptr, false,
    [this]() {
        m_oauth2->login();
    });
}

QList<Message> InoreaderNetworkFactory::decodeMessages(const QString &messages_json_data,
        const QString &stream_id)
{
    QList<Message> messages;
    QJsonArray json = QJsonDocument::fromJson(messages_json_data.toUtf8()).object()["items"].toArray();

    messages.reserve(json.count());

    for (const QJsonValue &obj : json) {
        auto message_obj = obj.toObject();
        Message message;

        message.m_title = message_obj["title"].toString();
        message.m_author = message_obj["author"].toString();
        message.m_created = QDateTime::fromSecsSinceEpoch(message_obj["published"].toInt(), Qt::UTC);
        message.m_createdFromFeed = true;
        message.m_customId = message_obj["id"].toString();

        auto alternates = message_obj["alternate"].toArray();
        auto enclosures = message_obj["enclosure"].toArray();
        auto categories = message_obj["categories"].toArray();

        for (const QJsonValue &alt : alternates) {
            auto alt_obj = alt.toObject();
            QString mime = alt_obj["type"].toString();
            QString href = alt_obj["href"].toString();

            if (mime == QL1S("text/html")) {
                message.m_url = href;
            } else {
                message.m_enclosures.append(Enclosure(href, mime));
            }
        }

        for (const QJsonValue &enc : enclosures) {
            auto enc_obj = enc.toObject();
            QString mime = enc_obj["type"].toString();
            QString href = enc_obj["href"].toString();

            message.m_enclosures.append(Enclosure(href, mime));
        }

        for (const QJsonValue &cat : categories) {
            QString category = cat.toString();

            if (category.contains(INOREADER_STATE_READ)) {
                message.m_isRead = !category.contains(INOREADER_STATE_READING_LIST);
            } else if (category.contains(INOREADER_STATE_IMPORTANT)) {
                message.m_isImportant = category.contains(INOREADER_STATE_IMPORTANT);
            }
        }

        message.m_contents = message_obj["summary"].toObject()["content"].toString();
        message.m_feedId = stream_id;

        messages.append(message);
    }

    return messages;
}

RootItem *InoreaderNetworkFactory::decodeFeedCategoriesData(const QString &categories,
        const QString &feeds, bool obtain_icons)
{
    auto *parent = new RootItem();
    QJsonArray json = QJsonDocument::fromJson(categories.toUtf8()).object()["tags"].toArray();
    QMap<QString, RootItem *> cats;

    cats.insert(QString(), parent);

    for (const QJsonValue &obj : json) {
        auto label = obj.toObject();
        QString label_id = label["id"].toString();

        if (label_id.contains(QSL("/label/"))) {
            // We have label (not "state").
            auto *category = new Category();

            category->setDescription(label["htmlUrl"].toString());
            category->setTitle(label_id.mid(label_id.lastIndexOf(QL1C('/')) + 1));
            category->setCustomId(label_id);

            cats.insert(category->customId(), category);
            parent->appendChild(category);
        }
    }

    json = QJsonDocument::fromJson(feeds.toUtf8()).object()["subscriptions"].toArray();

    for (const QJsonValue &obj : json) {
        auto subscription = obj.toObject();
        QString id = subscription["id"].toString();
        QString title = subscription["title"].toString();
        QString url = subscription["htmlUrl"].toString();
        QString parent_label;
        QJsonArray assigned_categories = subscription["categories"].toArray();

        for (const QJsonValue &cat : assigned_categories) {
            QString potential_id = cat.toObject()["id"].toString();

            if (potential_id.contains(QSL("/label/"))) {
                parent_label = potential_id;
                break;
            }
        }

        // We have label (not "state").
        auto *feed = new InoreaderFeed();

        feed->setDescription(url);
        feed->setUrl(url);
        feed->setTitle(title);
        feed->setCustomId(id);

        if (obtain_icons) {
            QString icon_url = subscription["iconUrl"].toString();

            if (!icon_url.isEmpty()) {
                QByteArray icon_data;

                if (NetworkFactory::performNetworkOperation(icon_url, DOWNLOAD_TIMEOUT,
                        QByteArray(), icon_data,
                        QNetworkAccessManager::GetOperation).first == QNetworkReply::NoError) {
                    // Icon downloaded, set it up.
                    QPixmap icon_pixmap;

                    icon_pixmap.loadFromData(icon_data);
                    feed->setIcon(QIcon(icon_pixmap));
                }
            }
        }

        if (cats.contains(parent_label)) {
            cats[parent_label]->appendChild(feed);
        }
    }

    return parent;
}
