// For license of this file, see <project-root-folder>/LICENSE.md.

#include "services/abstract/feed.h"

#include "definitions/definitions.h"
#include "miscellaneous/application.h"
#include "miscellaneous/databasequeries.h"
#include "miscellaneous/feedreader.h"
#include "miscellaneous/iconfactory.h"
#include "miscellaneous/mutex.h"
#include "miscellaneous/textfactory.h"
#include "services/abstract/cacheforserviceroot.h"
#include "services/abstract/importantnode.h"
#include "services/abstract/recyclebin.h"
#include "services/abstract/serviceroot.h"

#include <QThread>

Feed::Feed(RootItem *parent)
    : RootItem(parent), m_url(QString()), m_status(Status::Normal),
      m_autoUpdateType(AutoUpdateType::DefaultAutoUpdate),
      m_autoUpdateInitialInterval(DEFAULT_AUTO_UPDATE_INTERVAL),
      m_autoUpdateRemainingInterval(DEFAULT_AUTO_UPDATE_INTERVAL),
      m_messageFilters(QList<QPointer<MessageFilter>>())
{
    setKind(RootItem::Kind::Feed);
}

Feed::Feed(const QSqlRecord &record) : Feed(nullptr)
{
    setTitle(record.value(FDS_DB_TITLE_INDEX).toString());
    setId(record.value(FDS_DB_ID_INDEX).toInt());
    setUrl(record.value(FDS_DB_URL_INDEX).toString());
    setCustomId(record.value(FDS_DB_CUSTOM_ID_INDEX).toString());

    if (customId().isEmpty()) {
        setCustomId(QString::number(id()));
    }

    setDescription(QString::fromUtf8(record.value(FDS_DB_DESCRIPTION_INDEX).toByteArray()));
    setCreationDate(TextFactory::parseDateTime(record.value(
                        FDS_DB_DCREATED_INDEX).value<qint64>()).toLocalTime());
    setIcon(qApp->icons()->fromByteArray(record.value(FDS_DB_ICON_INDEX).toByteArray()));
    setAutoUpdateType(static_cast<Feed::AutoUpdateType>(record.value(
                          FDS_DB_UPDATE_TYPE_INDEX).toInt()));
    setAutoUpdateInitialInterval(record.value(FDS_DB_UPDATE_INTERVAL_INDEX).toInt());

    qDebug("Custom ID of feed when loading from DB is '%s'.", qPrintable(customId()));
}

Feed::Feed(const Feed &other) : RootItem(other)
{
    setKind(RootItem::Kind::Feed);

    setCountOfAllMessages(other.countOfAllMessages());
    setCountOfUnreadMessages(other.countOfUnreadMessages());
    setUrl(other.url());
    setStatus(other.status());
    setAutoUpdateType(other.autoUpdateType());
    setAutoUpdateInitialInterval(other.autoUpdateInitialInterval());
    setAutoUpdateRemainingInterval(other.autoUpdateRemainingInterval());
    setMessageFilters(other.messageFilters());
}

Feed::~Feed() = default;

QList<Message> Feed::undeletedMessages() const
{
    QSqlDatabase database = qApp->database()->connection(metaObject()->className());

    return DatabaseQueries::getUndeletedMessagesForFeed(database, customId(),
            getParentServiceRoot()->accountId());
}

QVariant Feed::data(int column, int role) const
{
    switch (role) {
        case Qt::ForegroundRole:
            switch (status()) {
                case Status::NewMessages:
                    return QColor(Qt::blue);

                case Status::NetworkError:
                case Status::ParsingError:
                case Status::AuthError:
                case Status::OtherError:
                    return QColor(Qt::red);

                default:
                    return QVariant();
            }

        default:
            return RootItem::data(column, role);
    }
}

int Feed::autoUpdateInitialInterval() const
{
    return m_autoUpdateInitialInterval;
}

int Feed::countOfAllMessages() const
{
    return m_totalCount;
}

int Feed::countOfUnreadMessages() const
{
    return m_unreadCount;
}

void Feed::setCountOfAllMessages(int count_all_messages)
{
    m_totalCount = count_all_messages;
}

void Feed::setCountOfUnreadMessages(int count_unread_messages)
{
    if (status() == Status::NewMessages && count_unread_messages < countOfUnreadMessages()) {
        setStatus(Status::Normal);
    }

    m_unreadCount = count_unread_messages;
}

void Feed::setAutoUpdateInitialInterval(int auto_update_interval)
{
    // If new initial auto-update interval is set, then
    // we should reset time that remains to the next auto-update.
    m_autoUpdateInitialInterval = auto_update_interval;
    m_autoUpdateRemainingInterval = auto_update_interval;
}

Feed::AutoUpdateType Feed::autoUpdateType() const
{
    return m_autoUpdateType;
}

void Feed::setAutoUpdateType(Feed::AutoUpdateType auto_update_type)
{
    m_autoUpdateType = auto_update_type;
}

int Feed::autoUpdateRemainingInterval() const
{
    return m_autoUpdateRemainingInterval;
}

void Feed::setAutoUpdateRemainingInterval(int auto_update_remaining_interval)
{
    m_autoUpdateRemainingInterval = auto_update_remaining_interval;
}

Feed::Status Feed::status() const
{
    return m_status;
}

void Feed::setStatus(const Feed::Status &status)
{
    m_status = status;
}

QString Feed::url() const
{
    return m_url;
}

void Feed::setUrl(const QString &url)
{
    m_url = url;
}

void Feed::appendMessageFilter(MessageFilter *filter)
{
    m_messageFilters.append(QPointer<MessageFilter>(filter));
}

void Feed::updateCounts(bool including_total_count)
{
    bool is_main_thread = QThread::currentThread() == qApp->thread();
    QSqlDatabase database = is_main_thread ?
                            qApp->database()->connection(metaObject()->className()) :
                            qApp->database()->connection(QSL("feed_upd"));
    int account_id = getParentServiceRoot()->accountId();

    if (including_total_count) {
        setCountOfAllMessages(DatabaseQueries::getMessageCountsForFeed(database, customId(), account_id,
                              true));
    }

    setCountOfUnreadMessages(DatabaseQueries::getMessageCountsForFeed(database, customId(), account_id,
                             false));
}

bool Feed::cleanMessages(bool clean_read_only)
{
    return getParentServiceRoot()->cleanFeeds(QList<Feed *>() << this, clean_read_only);
}

bool Feed::markAsReadUnread(RootItem::ReadStatus status)
{
    ServiceRoot *service = getParentServiceRoot();
    auto *cache = dynamic_cast<CacheForServiceRoot *>(service);

    if (cache != nullptr) {
        cache->addMessageStatesToCache(service->customIDSOfMessagesForItem(this), status);
    }

    return service->markFeedsReadUnread(QList<Feed *>() << this, status);
}

int Feed::updateMessages(const QList<Message> &messages, bool error_during_obtaining)
{
    QList<RootItem *> items_to_update;
    int updated_messages = 0;

    if (!error_during_obtaining) {
        bool is_main_thread = QThread::currentThread() == qApp->thread();

        qDebug("Updating messages in DB. Main thread: '%s'.",
               qPrintable(is_main_thread ? "true" : "false"));

        bool anything_updated = false;
        bool ok = true;

        if (!messages.isEmpty()) {
            qDebug("There are some messages to be updated/added to DB.");

            QString custom_id = customId();
            int account_id = getParentServiceRoot()->accountId();
            QSqlDatabase database = is_main_thread ?
                                    qApp->database()->connection(metaObject()->className()) :
                                    qApp->database()->connection(QSL("feed_upd"));

            updated_messages = DatabaseQueries::updateMessages(database, messages, custom_id, account_id, url(),
                               &anything_updated, &ok);
        } else {
            qWarning("There are no messages for update.");
        }

        if (ok) {
            setStatus(updated_messages > 0 ? Status::NewMessages : Status::Normal);
            updateCounts(true);

            if (getParentServiceRoot()->recycleBin() != nullptr && anything_updated) {
                getParentServiceRoot()->recycleBin()->updateCounts(true);
                items_to_update.append(getParentServiceRoot()->recycleBin());
            }

            if (getParentServiceRoot()->importantNode() != nullptr && anything_updated) {
                getParentServiceRoot()->importantNode()->updateCounts(true);
                items_to_update.append(getParentServiceRoot()->importantNode());
            }
        }
    } else {
        qCritical("There is indication that there was error during messages obtaining.");
    }

    // Some messages were really added to DB, reload feed in model.
    items_to_update.append(this);
    getParentServiceRoot()->itemChanged(items_to_update);

    return updated_messages;
}

QString Feed::getAutoUpdateStatusDescription() const
{
    QString auto_update_string;

    switch (autoUpdateType()) {
        case AutoUpdateType::DontAutoUpdate:

            //: Describes feed auto-update status.
            auto_update_string = tr("does not use auto-update");
            break;

        case AutoUpdateType::DefaultAutoUpdate:

            //: Describes feed auto-update status.
            auto_update_string = qApp->feedReader()->autoUpdateEnabled()
                                 ? tr("uses global settings (%n minute(s) to next auto-update)",
                                      nullptr,
                                      qApp->feedReader()->autoUpdateRemainingInterval())
                                 : tr("uses global settings (global feed auto-updating is disabled)");
            break;

        case AutoUpdateType::SpecificAutoUpdate:
        default:

            //: Describes feed auto-update status.
            auto_update_string = tr("uses specific settings (%n minute(s) to next auto-update)", nullptr,
                                    autoUpdateRemainingInterval());
            break;
    }

    return auto_update_string;
}

QString Feed::getStatusDescription() const
{
    switch (m_status) {
        case Status::Normal:
            return tr("no errors");

        case Status::NewMessages:
            return tr("has new messages");

        case Status::AuthError:
            return tr("authentication error");

        case Status::NetworkError:
            return tr("network error");

        default:
            return tr("unspecified error");
    }
}

QList<QPointer<MessageFilter>> Feed::messageFilters() const
{
    return m_messageFilters;
}

void Feed::setMessageFilters(const QList<QPointer<MessageFilter>> &filters)
{
    m_messageFilters = filters;
}

void Feed::removeMessageFilter(MessageFilter *filter)
{
    int idx = m_messageFilters.indexOf(filter);

    if (idx >= 0) {
        m_messageFilters.removeAll(filter);
    }
}

QString Feed::additionalTooltip() const
{
    return tr("Auto-update status: %1\n"
              "Active message filters: %2\n"
              "Status: %3").arg(getAutoUpdateStatusDescription(),
                                QString::number(m_messageFilters.size()),
                                getStatusDescription());
}
