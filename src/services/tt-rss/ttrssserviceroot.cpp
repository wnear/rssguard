// This file is part of RSS Guard.
//
// Copyright (C) 2011-2015 by Martin Rotter <rotter.martinos@gmail.com>
//
// RSS Guard is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// RSS Guard is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with RSS Guard. If not, see <http://www.gnu.org/licenses/>.

#include "services/tt-rss/ttrssserviceroot.h"

#include "miscellaneous/application.h"
#include "miscellaneous/settings.h"
#include "gui/dialogs/formmain.h"
#include "network-web/networkfactory.h"
#include "services/tt-rss/ttrssserviceentrypoint.h"
#include "services/tt-rss/ttrssfeed.h"
#include "services/tt-rss/ttrsscategory.h"
#include "services/tt-rss/network/ttrssnetworkfactory.h"
#include "services/tt-rss/gui/formeditaccount.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QPointer>
#include <QPair>


TtRssServiceRoot::TtRssServiceRoot(RootItem *parent)
  : ServiceRoot(parent), m_actionSyncIn(NULL), m_serviceMenu(QList<QAction*>()), m_network(new TtRssNetworkFactory) {
  setIcon(TtRssServiceEntryPoint().icon());
  setCreationDate(QDateTime::currentDateTime());
}

TtRssServiceRoot::~TtRssServiceRoot() {
  if (m_network != NULL) {
    delete m_network;
  }
}

void TtRssServiceRoot::start() {
  loadFromDatabase();

  if (childCount() == 0) {
    syncIn();
  }
}

void TtRssServiceRoot::stop() {
  QNetworkReply::NetworkError error;
  m_network->logout(error);

  qDebug("Stopping Tiny Tiny RSS account, logging out with result '%d'.", (int) error);
}

QString TtRssServiceRoot::code() {
  return SERVICE_CODE_TT_RSS;
}

bool TtRssServiceRoot::editViaGui() {
  QPointer<FormEditAccount> form_pointer = new FormEditAccount(qApp->mainForm());
  form_pointer.data()->execForEdit(this);
  delete form_pointer.data();
  return false;
}

bool TtRssServiceRoot::deleteViaGui() {
  QSqlDatabase connection = qApp->database()->connection(metaObject()->className(), DatabaseFactory::FromSettings);

  // Remove extra entry in "Tiny Tiny RSS accounts list" and then delete
  // all the categories/feeds and messages.
  if (!QSqlQuery(connection).exec(QString("DELETE FROM TtRssAccounts WHERE id = %1;").arg(accountId()))) {
    return false;
  }
  else {
    return ServiceRoot::deleteViaGui();
  }
}

bool TtRssServiceRoot::canBeEdited() {
  return true;
}

bool TtRssServiceRoot::canBeDeleted() {
  return true;
}

QVariant TtRssServiceRoot::data(int column, int role) const {
  switch (role) {
    case Qt::ToolTipRole:
      if (column == FDS_MODEL_TITLE_INDEX) {
        return tr("Tiny Tiny RSS\n\nAccount ID: %3\nUsername: %1\nServer: %2").arg(m_network->username(),
                                                                                   m_network->url(),
                                                                                   QString::number(accountId()));
      }
      else {
        return ServiceRoot::data(column, role);
      }

    default:
      return ServiceRoot::data(column, role);
  }
}

QList<QAction*> TtRssServiceRoot::addItemMenu() {
  return QList<QAction*>();
}

RecycleBin *TtRssServiceRoot::recycleBin() {
  return NULL;
}

bool TtRssServiceRoot::loadMessagesForItem(RootItem *item, QSqlTableModel *model) {
  return false;
}

QList<QAction*> TtRssServiceRoot::serviceMenu() {
  if (m_serviceMenu.isEmpty()) {
    m_actionSyncIn = new QAction(qApp->icons()->fromTheme(QSL("item-sync")), tr("Sync in"), this);

    connect(m_actionSyncIn, SIGNAL(triggered()), this, SLOT(syncIn()));

    m_serviceMenu.append(m_actionSyncIn);
  }

  return m_serviceMenu;
}

QList<QAction*> TtRssServiceRoot::contextMenu() {
  return serviceMenu();
}

bool TtRssServiceRoot::onBeforeSetMessagesRead(RootItem *selected_item, QList<int> message_db_ids, RootItem::ReadStatus read) {
  return false;
}

bool TtRssServiceRoot::onAfterSetMessagesRead(RootItem *selected_item, QList<int> message_db_ids, RootItem::ReadStatus read) {
  return false;
}

bool TtRssServiceRoot::onBeforeSwitchMessageImportance(RootItem *selected_item, QList<QPair<int, RootItem::Importance> > changes) {
  return false;
}

bool TtRssServiceRoot::onAfterSwitchMessageImportance(RootItem *selected_item, QList<QPair<int, RootItem::Importance> > changes) {
  return false;
}

bool TtRssServiceRoot::onBeforeMessagesDelete(RootItem *selected_item, QList<int> message_db_ids) {
  return false;
}

bool TtRssServiceRoot::onAfterMessagesDelete(RootItem *selected_item, QList<int> message_db_ids) {
  return false;
}

bool TtRssServiceRoot::onBeforeMessagesRestoredFromBin(RootItem *selected_item, QList<int> message_db_ids) {
  return false;
}

bool TtRssServiceRoot::onAfterMessagesRestoredFromBin(RootItem *selected_item, QList<int> message_db_ids) {
  return false;
}

TtRssNetworkFactory *TtRssServiceRoot::network() const {
  return m_network;
}

void TtRssServiceRoot::saveAccountDataToDatabase() {
  if (accountId() != NO_PARENT_CATEGORY) {
    // We are overwritting previously saved data.
    QSqlDatabase database = qApp->database()->connection(metaObject()->className(), DatabaseFactory::FromSettings);
    QSqlQuery query(database);

    query.prepare("UPDATE TtRssAccounts "
                  "SET username = :username, password = :password, url = :url "
                  "WHERE id = :id;");
    query.bindValue(":username", m_network->username());
    query.bindValue(":password", m_network->password());
    query.bindValue(":url", m_network->url());
    query.bindValue(":id", accountId());

    if (query.exec()) {
      updateTitle();
      itemChanged(QList<RootItem*>() << this);
    }
  }
  else {
    // We are probably saving newly added account.
    QSqlDatabase database = qApp->database()->connection(metaObject()->className(), DatabaseFactory::FromSettings);
    QSqlQuery query(database);

    // First obtain the ID, which can be assigned to this new account.
    if (!query.exec("SELECT max(id) FROM Accounts;") || !query.next()) {
      return;
    }

    int id_to_assing = query.value(0).toInt() + 1;

    bool saved = query.exec(QString("INSERT INTO Accounts (id, type) VALUES (%1, '%2');").arg(QString::number(id_to_assing),
                                                                                              SERVICE_CODE_TT_RSS)) &&
                 query.exec(QString("INSERT INTO TtRssAccounts (id, username, password, url) VALUES (%1, '%2', '%3', '%4');").arg(QString::number(id_to_assing),
                                                                                                                                  network()->username(),
                                                                                                                                  network()->password(),
                                                                                                                                  network()->url()));

    if (saved) {
      setAccountId(id_to_assing);
      updateTitle();
    }
  }
}

void TtRssServiceRoot::loadFromDatabase() {
  // TODO: Load feeds/categories from DB.

  QSqlDatabase database = qApp->database()->connection(metaObject()->className(), DatabaseFactory::FromSettings);
  Assignment categories;
  Assignment feeds;

  // Obtain data for categories from the database.
  QSqlQuery query_categories(database);
  query_categories.setForwardOnly(true);

  if (!query_categories.exec(QString("SELECT * FROM Categories WHERE account_id = %1;").arg(accountId())) || query_categories.lastError().isValid()) {
    qFatal("Query for obtaining categories failed. Error message: '%s'.",
           qPrintable(query_categories.lastError().text()));
  }

  while (query_categories.next()) {
    AssignmentItem pair;
    pair.first = query_categories.value(CAT_DB_PARENT_ID_INDEX).toInt();
    pair.second = new TtRssCategory(query_categories.record());

    categories << pair;
  }

  // All categories are now loaded.
  QSqlQuery query_feeds(database);
  query_feeds.setForwardOnly(true);

  if (!query_feeds.exec(QString("SELECT * FROM Feeds WHERE account_id = %1;").arg(accountId())) || query_feeds.lastError().isValid()) {
    qFatal("Query for obtaining feeds failed. Error message: '%s'.",
           qPrintable(query_feeds.lastError().text()));
  }

  while (query_feeds.next()) {
    AssignmentItem pair;
    pair.first = query_feeds.value(FDS_DB_CATEGORY_INDEX).toInt();
    pair.second = new TtRssFeed(query_feeds.record());

    feeds << pair;
  }

  // All data are now obtained, lets create the hierarchy.
  assembleCategories(categories);
  assembleFeeds(feeds);
}

void TtRssServiceRoot::updateTitle() {
  QString host = QUrl(m_network->url()).host();

  if (host.isEmpty()) {
    host = m_network->url();
  }

  setTitle(m_network->username() + QL1S("@") + host);
}

void TtRssServiceRoot::syncIn() {
  QNetworkReply::NetworkError err;
  TtRssGetFeedsCategoriesResponse feed_cats_response = m_network->getFeedsCategories(err);

  if (err == QNetworkReply::NoError) {
    RootItem *new_tree = feed_cats_response.feedsCategories(true, m_network->url());

    // Purge old data from SQL and clean all model items.
    removeOldFeedTree();
    cleanAllItems();

    // Model is clean, now store new tree into DB and
    // set primary IDs of the items.
    storeNewFeedTree(new_tree);

    foreach (RootItem *top_level_item, new_tree->childItems()) {
      appendChild(top_level_item);
    }

    updateCounts(true);

    new_tree->clearChildren();
    new_tree->deleteLater();

    itemChanged(QList<RootItem*>() << this);
    requestFeedReadFilterReload();
    requestReloadMessageList(true);
    requestItemExpand(getSubTree(), true);
  }
}

void TtRssServiceRoot::removeOldFeedTree() {
  QSqlDatabase database = qApp->database()->connection(metaObject()->className(), DatabaseFactory::FromSettings);
  QSqlQuery query(database);
  query.setForwardOnly(true);

  query.prepare(QSL("DELETE FROM Feeds WHERE account_id = :account_id;"));
  query.bindValue(QSL(":account_id"), accountId());
  query.exec();

  query.prepare(QSL("DELETE FROM Categories WHERE account_id = :account_id;"));
  query.bindValue(QSL(":account_id"), accountId());
  query.exec();
}

void TtRssServiceRoot::cleanAllItems() {
  foreach (RootItem *top_level_item, childItems()) {
    requestItemRemoval(top_level_item);
  }
}

void TtRssServiceRoot::storeNewFeedTree(RootItem *root) {
  QSqlDatabase database = qApp->database()->connection(metaObject()->className(), DatabaseFactory::FromSettings);
  QSqlQuery query_category(database);
  QSqlQuery query_feed(database);

  query_category.prepare("INSERT INTO Categories (parent_id, title, account_id, custom_id) "
                         "VALUES (:parent_id, :title, :account_id, :custom_id);");
  query_feed.prepare("INSERT INTO Feeds (title, icon, category, protected, update_type, update_interval, account_id, custom_id) "
                     "VALUES (:title, :icon, :category, :protected, :update_type, :update_interval, :account_id, :custom_id);");

  // Iterate all children.
  foreach (RootItem *child, root->getSubTree()) {
    if (child->kind() == RootItemKind::Category) {
      query_category.bindValue(QSL(":parent_id"), child->parent()->id());
      query_category.bindValue(QSL(":title"), child->title());
      query_category.bindValue(QSL(":account_id"), accountId());
      query_category.bindValue(QSL(":custom_id"), QString::number(static_cast<TtRssCategory*>(child)->customId()));

      if (query_category.exec()) {
        child->setId(query_category.lastInsertId().toInt());
      }
      else {
        // TODO: logovat
      }
    }
    else if (child->kind() == RootItemKind::Feed) {
      TtRssFeed *feed = static_cast<TtRssFeed*>(child);

      query_feed.bindValue(QSL(":title"), feed->title());
      query_feed.bindValue(QSL(":icon"), qApp->icons()->toByteArray(feed->icon()));
      query_feed.bindValue(QSL(":category"), feed->parent()->id());
      query_feed.bindValue(QSL(":protected"), 0);
      query_feed.bindValue(QSL(":update_type"), (int) feed->autoUpdateType());
      query_feed.bindValue(QSL(":update_interval"), feed->autoUpdateInitialInterval());
      query_feed.bindValue(QSL(":account_id"), accountId());
      query_feed.bindValue(QSL(":custom_id"), feed->customId());

      if (query_feed.exec()) {
        feed->setId(query_feed.lastInsertId().toInt());
      }
      else {
        // TODO: logovat.
      }
    }
  }
}
