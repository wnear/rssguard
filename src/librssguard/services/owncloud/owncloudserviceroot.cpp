// For license of this file, see <project-root-folder>/LICENSE.md.

#include "services/owncloud/owncloudserviceroot.h"

#include "definitions/definitions.h"
#include "miscellaneous/application.h"
#include "miscellaneous/databasequeries.h"
#include "miscellaneous/iconfactory.h"
#include "miscellaneous/mutex.h"
#include "miscellaneous/textfactory.h"
#include "services/abstract/importantnode.h"
#include "services/abstract/recyclebin.h"
#include "services/owncloud/gui/formeditowncloudaccount.h"
#include "services/owncloud/gui/formowncloudfeeddetails.h"
#include "services/owncloud/network/owncloudnetworkfactory.h"
#include "services/owncloud/owncloudfeed.h"
#include "services/owncloud/owncloudserviceentrypoint.h"

OwnCloudServiceRoot::OwnCloudServiceRoot(RootItem *parent)
    : ServiceRoot(parent), m_network(new OwnCloudNetworkFactory())
{
    setIcon(OwnCloudServiceEntryPoint().icon());
}

OwnCloudServiceRoot::~OwnCloudServiceRoot()
{
    delete m_network;
}

bool OwnCloudServiceRoot::isSyncable() const
{
    return true;
}

bool OwnCloudServiceRoot::canBeEdited() const
{
    return true;
}

bool OwnCloudServiceRoot::canBeDeleted() const
{
    return true;
}

bool OwnCloudServiceRoot::editViaGui()
{
    QScopedPointer<FormEditOwnCloudAccount> form_pointer(new FormEditOwnCloudAccount(
                qApp->mainFormWidget()));

    form_pointer.data()->execForEdit(this);
    return true;
}

bool OwnCloudServiceRoot::deleteViaGui()
{
    QSqlDatabase database = qApp->database()->connection(metaObject()->className());

    if (DatabaseQueries::deleteOwnCloudAccount(database, accountId())) {
        return ServiceRoot::deleteViaGui();
    } else {
        return false;
    }
}

bool OwnCloudServiceRoot::supportsFeedAdding() const
{
    return true;
}

bool OwnCloudServiceRoot::supportsCategoryAdding() const
{
    return false;
}

void OwnCloudServiceRoot::start(bool freshly_activated)
{
    Q_UNUSED(freshly_activated)
    loadFromDatabase();
    loadCacheFromFile(accountId());

    if (childCount() <= 2) {
        syncIn();
    }
}

void OwnCloudServiceRoot::stop()
{
    saveCacheToFile(accountId());
}

QString OwnCloudServiceRoot::code() const
{
    return OwnCloudServiceEntryPoint().code();
}

OwnCloudNetworkFactory *OwnCloudServiceRoot::network() const
{
    return m_network;
}

void OwnCloudServiceRoot::saveAllCachedData(bool async)
{
    QPair<QMap<RootItem::ReadStatus, QStringList>, QMap<RootItem::Importance, QList<Message>>> msgCache
        = takeMessageCache();
    QMapIterator<RootItem::ReadStatus, QStringList> i(msgCache.first);

    // Save the actual data read/unread.
    while (i.hasNext()) {
        i.next();
        auto key = i.key();
        QStringList ids = i.value();

        if (!ids.isEmpty()) {
            network()->markMessagesRead(key, ids, async);
        }
    }

    QMapIterator<RootItem::Importance, QList<Message>> j(msgCache.second);

    // Save the actual data important/not important.
    while (j.hasNext()) {
        j.next();
        auto key = j.key();
        QList<Message> messages = j.value();

        if (!messages.isEmpty()) {
            QStringList feed_ids, guid_hashes;

            for (const Message &msg : messages) {
                feed_ids.append(msg.m_feedId);
                guid_hashes.append(msg.m_customHash);
            }

            network()->markMessagesStarred(key, feed_ids, guid_hashes, async);
        }
    }
}

void OwnCloudServiceRoot::updateTitle()
{
    setTitle(m_network->authUsername() + QSL(" (Nextcloud News)"));
}

void OwnCloudServiceRoot::saveAccountDataToDatabase()
{
    QSqlDatabase database = qApp->database()->connection(metaObject()->className());

    if (accountId() != NO_PARENT_CATEGORY) {
        if (DatabaseQueries::overwriteOwnCloudAccount(database, m_network->authUsername(),
                m_network->authPassword(), m_network->url(),
                m_network->forceServerSideUpdate(), m_network->batchSize(),
                m_network->downloadOnlyUnreadMessages(), accountId())) {
            updateTitle();
            itemChanged(QList<RootItem *>() << this);
        }
    } else {
        bool saved;
        int id_to_assign = DatabaseQueries::createAccount(database, code(), &saved);

        if (saved) {
            if (DatabaseQueries::createOwnCloudAccount(database, id_to_assign, m_network->authUsername(),
                    m_network->authPassword(), m_network->url(),
                    m_network->forceServerSideUpdate(),
                    m_network->downloadOnlyUnreadMessages(),
                    m_network->batchSize())) {
                setId(id_to_assign);
                setAccountId(id_to_assign);
                updateTitle();
            }
        }
    }
}

void OwnCloudServiceRoot::addNewFeed(const QString &url)
{
    if (!qApp->feedUpdateLock()->tryLock()) {
        // Lock was not obtained because
        // it is used probably by feed updater or application
        // is quitting.
        qApp->showGuiMessage(tr("Cannot add item"),
                             tr("Cannot add feed because another critical operation is ongoing."),
                             QSystemTrayIcon::Warning, qApp->mainFormWidget(), true);

        // Thus, cannot delete and quit the method.
        return;
    }

    QScopedPointer<FormOwnCloudFeedDetails> form_pointer(new FormOwnCloudFeedDetails(this,
            qApp->mainFormWidget()));

    form_pointer.data()->addEditFeed(nullptr, this, url);
    qApp->feedUpdateLock()->unlock();
}

void OwnCloudServiceRoot::addNewCategory() {}

RootItem *OwnCloudServiceRoot::obtainNewTreeForSyncIn() const
{
    OwnCloudGetFeedsCategoriesResponse feed_cats_response = m_network->feedsCategories();

    if (m_network->lastError() == QNetworkReply::NoError) {
        return feed_cats_response.feedsCategories(true);
    } else {
        return nullptr;
    }
}

void OwnCloudServiceRoot::loadFromDatabase()
{
    QSqlDatabase database = qApp->database()->connection(metaObject()->className());
    Assignment categories = DatabaseQueries::getCategories<Category>(database, accountId());
    Assignment feeds = DatabaseQueries::getFeeds<OwnCloudFeed>(database,
                       qApp->feedReader()->messageFilters(), accountId());

    // All data are now obtained, lets create the hierarchy.
    assembleCategories(categories);
    assembleFeeds(feeds);

    // As the last item, add recycle bin, which is needed.
    appendChild(recycleBin());
    appendChild(importantNode());
    updateCounts(true);
}
