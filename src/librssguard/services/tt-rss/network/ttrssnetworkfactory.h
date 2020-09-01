// For license of this file, see <project-root-folder>/LICENSE.md.

#ifndef TTRSSNETWORKFACTORY_H
#define TTRSSNETWORKFACTORY_H

#include "core/message.h"

#include <QJsonObject>
#include <QNetworkReply>
#include <QPair>
#include <QString>

class RootItem;

class TtRssFeed;

class TtRssResponse
{
public:
    explicit TtRssResponse(const QString &raw_content = QString());
    virtual ~TtRssResponse();

    bool isLoaded() const;

    int seq() const;
    int status() const;
    QString error() const;
    bool hasError() const;
    bool isNotLoggedIn() const;
    QString toString() const;

protected:
    QJsonObject m_rawContent;
};

class TtRssLoginResponse : public TtRssResponse
{
public:
    explicit TtRssLoginResponse(const QString &raw_content = QString());
    virtual ~TtRssLoginResponse();

    int apiLevel() const;
    QString sessionId() const;
};

class TtRssGetFeedsCategoriesResponse : public TtRssResponse
{
public:
    explicit TtRssGetFeedsCategoriesResponse(const QString &raw_content = QString());
    virtual ~TtRssGetFeedsCategoriesResponse();

    // Returns tree of feeds/categories.
    // Top-level root of the tree is not needed here.
    // Returned items do not have primary IDs assigned.
    RootItem *feedsCategories(bool obtain_icons, QString base_address = QString()) const;
};

class TtRssGetHeadlinesResponse : public TtRssResponse
{
public:
    explicit TtRssGetHeadlinesResponse(const QString &raw_content = QString());
    virtual ~TtRssGetHeadlinesResponse();

    QList<Message> messages() const;
};

class TtRssUpdateArticleResponse : public TtRssResponse
{
public:
    explicit TtRssUpdateArticleResponse(const QString &raw_content = QString());
    virtual ~TtRssUpdateArticleResponse();

    QString updateStatus() const;
    int articlesUpdated() const;
};

class TtRssSubscribeToFeedResponse : public TtRssResponse
{
public:
    explicit TtRssSubscribeToFeedResponse(const QString &raw_content = QString());
    virtual ~TtRssSubscribeToFeedResponse();

    int code() const;
};

class TtRssUnsubscribeFeedResponse : public TtRssResponse
{
public:
    explicit TtRssUnsubscribeFeedResponse(const QString &raw_content = QString());
    virtual ~TtRssUnsubscribeFeedResponse();

    QString code() const;
};

namespace UpdateArticle
{
enum class Mode {
    SetToFalse = 0,
    SetToTrue = 1,
    Togggle = 2
};
enum class OperatingField {
    Starred = 0,
    Published = 1,
    Unread = 2
};
}

class TtRssNetworkFactory
{
public:
    explicit TtRssNetworkFactory();
    virtual ~TtRssNetworkFactory() = default;

    QString url() const;
    void setUrl(const QString &url);

    QString username() const;
    void setUsername(const QString &username);

    QString password() const;
    void setPassword(const QString &password);

    bool authIsUsed() const;
    void setAuthIsUsed(bool auth_is_used);

    QString authUsername() const;
    void setAuthUsername(const QString &auth_username);

    QString authPassword() const;
    void setAuthPassword(const QString &auth_password);

    bool forceServerSideUpdate() const;
    void setForceServerSideUpdate(bool force_server_side_update);

    bool downloadOnlyUnreadMessages() const;
    void setDownloadOnlyUnreadMessages(bool download_only_unread_messages);

    // Metadata.
    QDateTime lastLoginTime() const;
    QNetworkReply::NetworkError lastError() const;

    // Operations.

    // Logs user in.
    TtRssLoginResponse login();

    // Logs user out.
    TtRssResponse logout();

    // Gets feeds from the server.
    TtRssGetFeedsCategoriesResponse getFeedsCategories();

    // Gets headlines (messages) from the server.
    TtRssGetHeadlinesResponse getHeadlines(int feed_id, int limit, int skip,
                                           bool show_content, bool include_attachments,
                                           bool sanitize, bool unread_only);

    TtRssUpdateArticleResponse updateArticles(const QStringList &ids,
            UpdateArticle::OperatingField field,
            UpdateArticle::Mode mode, bool async = true);

    TtRssSubscribeToFeedResponse subscribeToFeed(const QString &url, int category_id,
            bool protectd = false,
            const QString &username = QString(), const QString &password = QString());

    TtRssUnsubscribeFeedResponse unsubscribeFeed(int feed_id);

private:
    QString m_bareUrl;
    QString m_fullUrl;
    QString m_username;
    QString m_password;
    bool m_forceServerSideUpdate;
    bool m_downloadOnlyUnreadMessages;
    bool m_authIsUsed;
    QString m_authUsername;
    QString m_authPassword;
    QString m_sessionId;
    QDateTime m_lastLoginTime;

    QNetworkReply::NetworkError m_lastError;
};

#endif // TTRSSNETWORKFACTORY_H
