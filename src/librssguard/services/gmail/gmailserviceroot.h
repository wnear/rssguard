// For license of this file, see <project-root-folder>/LICENSE.md.

#ifndef GMAILSERVICEROOT_H
#define GMAILSERVICEROOT_H

#include "services/abstract/cacheforserviceroot.h"
#include "services/abstract/serviceroot.h"

class GmailNetworkFactory;

class GmailServiceRoot : public ServiceRoot, public CacheForServiceRoot
{
    Q_OBJECT

public:
    explicit GmailServiceRoot(GmailNetworkFactory *network, RootItem *parent = nullptr);
    virtual ~GmailServiceRoot();

    void saveAccountDataToDatabase();

    bool downloadAttachmentOnMyOwn(const QUrl &url) const;

    void setNetwork(GmailNetworkFactory *network);
    GmailNetworkFactory *network() const;

    QList<QAction *> contextMenuMessagesList(const QList<Message> &messages);
    QList<QAction *> serviceMenu();
    bool isSyncable() const;
    bool canBeEdited() const;
    bool editViaGui();
    bool canBeDeleted() const;
    bool deleteViaGui();
    bool supportsFeedAdding() const;
    bool supportsCategoryAdding() const;
    void start(bool freshly_activated);
    void stop();
    QString code() const;

    QString additionalTooltip() const;

    void saveAllCachedData(bool async = true);

public slots:
    void updateTitle();

private slots:
    void replyToEmail();

protected:
    RootItem *obtainNewTreeForSyncIn() const;

private:
    void writeNewEmail();
    void loadFromDatabase();

private:
    GmailNetworkFactory *m_network;
    QAction *m_actionReply;
    Message m_replyToMessage;
};

inline void GmailServiceRoot::setNetwork(GmailNetworkFactory *network)
{
    m_network = network;
}

inline GmailNetworkFactory *GmailServiceRoot::network() const
{
    return m_network;
}

#endif // GMAILSERVICEROOT_H
