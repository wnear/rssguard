// For license of this file, see <project-root-folder>/LICENSE.md.

#include "services/inoreader/inoreaderserviceroot.h"

#include "miscellaneous/application.h"
#include "miscellaneous/databasequeries.h"
#include "miscellaneous/iconfactory.h"
#include "network-web/oauth2service.h"
#include "services/abstract/importantnode.h"
#include "services/abstract/recyclebin.h"
#include "services/inoreader/gui/formeditinoreaderaccount.h"
#include "services/inoreader/inoreaderentrypoint.h"
#include "services/inoreader/inoreaderfeed.h"
#include "services/inoreader/network/inoreadernetworkfactory.h"

InoreaderServiceRoot::InoreaderServiceRoot(InoreaderNetworkFactory *network, RootItem *parent)
    : ServiceRoot(parent), m_network(network)
{
    if (network == nullptr) {
        m_network = new InoreaderNetworkFactory(this);
    } else {
        m_network->setParent(this);
    }

    m_network->setService(this);
    setIcon(InoreaderEntryPoint().icon());
}

InoreaderServiceRoot::~InoreaderServiceRoot() = default;

void InoreaderServiceRoot::updateTitle()
{
    setTitle(m_network->userName() + QSL(" (Inoreader)"));
}

void InoreaderServiceRoot::loadFromDatabase()
{
    QSqlDatabase database = qApp->database()->connection(metaObject()->className());
    Assignment categories = DatabaseQueries::getCategories<Category>(database, accountId());
    Assignment feeds = DatabaseQueries::getFeeds<InoreaderFeed>(database,
                       qApp->feedReader()->messageFilters(), accountId());

    // All data are now obtained, lets create the hierarchy.
    assembleCategories(categories);
    assembleFeeds(feeds);

    // As the last item, add recycle bin, which is needed.
    appendChild(recycleBin());
    appendChild(importantNode());
    updateCounts(true);
}

void InoreaderServiceRoot::saveAccountDataToDatabase()
{
    QSqlDatabase database = qApp->database()->connection(metaObject()->className());

    if (accountId() != NO_PARENT_CATEGORY) {
        if (DatabaseQueries::overwriteInoreaderAccount(database, m_network->userName(),
                m_network->oauth()->clientId(),
                m_network->oauth()->clientSecret(),
                m_network->oauth()->redirectUrl(),
                m_network->oauth()->refreshToken(),
                m_network->batchSize(),
                accountId())) {
            updateTitle();
            itemChanged(QList<RootItem *>() << this);
        }
    } else {
        bool saved;
        int id_to_assign = DatabaseQueries::createAccount(database, code(), &saved);

        if (saved) {
            if (DatabaseQueries::createInoreaderAccount(database, id_to_assign,
                    m_network->userName(),
                    m_network->oauth()->clientId(),
                    m_network->oauth()->clientSecret(),
                    m_network->oauth()->redirectUrl(),
                    m_network->oauth()->refreshToken(),
                    m_network->batchSize())) {
                setId(id_to_assign);
                setAccountId(id_to_assign);
                updateTitle();
            }
        }
    }
}

bool InoreaderServiceRoot::isSyncable() const
{
    return true;
}

bool InoreaderServiceRoot::canBeEdited() const
{
    return true;
}

bool InoreaderServiceRoot::editViaGui()
{
    FormEditInoreaderAccount form_pointer(qApp->mainFormWidget());

    form_pointer.execForEdit(this);
    return true;
}

bool InoreaderServiceRoot::supportsFeedAdding() const
{
    return false;
}

bool InoreaderServiceRoot::supportsCategoryAdding() const
{
    return false;
}

void InoreaderServiceRoot::start(bool freshly_activated)
{
    Q_UNUSED(freshly_activated)

    loadFromDatabase();
    loadCacheFromFile(accountId());

    if (childCount() <= 2) {
        syncIn();
    } else {
        m_network->oauth()->login();
    }
}

void InoreaderServiceRoot::stop()
{
    saveCacheToFile(accountId());
}

QString InoreaderServiceRoot::code() const
{
    return InoreaderEntryPoint().code();
}

QString InoreaderServiceRoot::additionalTooltip() const
{
    return tr("Authentication status: %1\n"
              "Login tokens expiration: %2").arg(network()->oauth()->isFullyLoggedIn() ? tr("logged-in") :
                      tr("NOT logged-in"),
                      network()->oauth()->tokensExpireIn().isValid() ?
                      network()->oauth()->tokensExpireIn().toString() : QSL("-"));
}

RootItem *InoreaderServiceRoot::obtainNewTreeForSyncIn() const
{
    return m_network->feedsCategories(true);
}

void InoreaderServiceRoot::addNewFeed(const QString &url)
{
    Q_UNUSED(url)
}

void InoreaderServiceRoot::addNewCategory() {}

void InoreaderServiceRoot::saveAllCachedData(bool async)
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
            QStringList custom_ids;

            for (const Message &msg : messages) {
                custom_ids.append(msg.m_customId);
            }

            network()->markMessagesStarred(key, custom_ids, async);
        }
    }
}

bool InoreaderServiceRoot::canBeDeleted() const
{
    return true;
}

bool InoreaderServiceRoot::deleteViaGui()
{
    QSqlDatabase database = qApp->database()->connection(metaObject()->className());

    if (DatabaseQueries::deleteInoreaderAccount(database, accountId())) {
        return ServiceRoot::deleteViaGui();
    } else {
        return false;
    }
}
