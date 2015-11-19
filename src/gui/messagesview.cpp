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

#include "gui/messagesview.h"

#include "core/messagesproxymodel.h"
#include "core/messagesmodel.h"
#include "miscellaneous/settings.h"
#include "network-web/networkfactory.h"
#include "network-web/webfactory.h"
#include "gui/dialogs/formmain.h"
#include "gui/messagebox.h"
#include "gui/styleditemdelegatewithoutfocus.h"

#include <QKeyEvent>
#include <QScrollBar>
#include <QMenu>


MessagesView::MessagesView(QWidget *parent)
  : QTreeView(parent),
    m_contextMenu(NULL),
    m_columnsAdjusted(false),
    m_batchUnreadSwitch(false) {
  m_proxyModel = new MessagesProxyModel(this);
  m_sourceModel = m_proxyModel->sourceModel();

  // Forward count changes to the view.
  createConnections();
  setModel(m_proxyModel);
  setupAppearance();
}

MessagesView::~MessagesView() {
  qDebug("Destroying MessagesView instance.");
}

void MessagesView::setSortingEnabled(bool enable) {
  disconnect(header(), SIGNAL(sortIndicatorChanged(int,Qt::SortOrder)), this, SLOT(saveSortState(int,Qt::SortOrder)));
  QTreeView::setSortingEnabled(enable);
  connect(header(), SIGNAL(sortIndicatorChanged(int,Qt::SortOrder)), this, SLOT(saveSortState(int,Qt::SortOrder)));
}

void MessagesView::createConnections() {
  connect(this, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(openSelectedSourceMessagesInternallyNoNewTab()));

  // Adjust columns when layout gets changed.
  connect(header(), SIGNAL(geometriesChanged()), this, SLOT(adjustColumns()));
  connect(header(), SIGNAL(sortIndicatorChanged(int,Qt::SortOrder)), this, SLOT(saveSortState(int,Qt::SortOrder)));
}

void MessagesView::keyboardSearch(const QString &search) {
  // WARNING: This is quite hacky way how to force selection of next item even
  // with extended selection enabled.
  setSelectionMode(QAbstractItemView::SingleSelection);
  QTreeView::keyboardSearch(search);
  setSelectionMode(QAbstractItemView::ExtendedSelection);
}

void MessagesView::reloadSelections(bool mark_current_index_read) {
  QModelIndex current_index = selectionModel()->currentIndex();
  QModelIndex mapped_current_index = m_proxyModel->mapToSource(current_index);
  QModelIndexList selected_indexes = selectionModel()->selectedRows();
  QModelIndexList mapped_indexes = m_proxyModel->mapListToSource(selected_indexes);

  // Reload the model now.
  m_sourceModel->fetchAllData();

  sortByColumn(header()->sortIndicatorSection(), header()->sortIndicatorOrder());

  selected_indexes = m_proxyModel->mapListFromSource(mapped_indexes, true);
  current_index = m_proxyModel->mapFromSource(m_sourceModel->index(mapped_current_index.row(), mapped_current_index.column()));

  if (current_index.isValid()) {
    if (!mark_current_index_read) {
      // User selected to mark some messages as unread, if one
      // of them will be marked as current, then it will be read again.
      m_batchUnreadSwitch = true;
    }

    setCurrentIndex(current_index);
    scrollTo(current_index);
    reselectIndexes(selected_indexes);
    m_batchUnreadSwitch = false;
  }
  else {
    // Messages were probably removed from the model, nothing can
    // be selected and no message can be displayed.
    // TOTO: Check if this is OKAY. If not, then emit this signal
    // from FeedsView itself.
    emit currentMessagesRemoved();
  }
}

void MessagesView::setupAppearance() {
  setUniformRowHeights(true);
  setAcceptDrops(false);
  setDragEnabled(false);
  setDragDropMode(QAbstractItemView::NoDragDrop);
  setExpandsOnDoubleClick(false);
  setRootIsDecorated(false);
  setEditTriggers(QAbstractItemView::NoEditTriggers);
  setItemsExpandable(false);
  setSortingEnabled(true);
  setAllColumnsShowFocus(false);
  setSelectionMode(QAbstractItemView::ExtendedSelection);
  setItemDelegate(new StyledItemDelegateWithoutFocus(this));
  header()->setDefaultSectionSize(MESSAGES_VIEW_DEFAULT_COL);
  header()->setStretchLastSection(false);
  header()->setSortIndicatorShown(false);
}

void MessagesView::keyPressEvent(QKeyEvent *event) {
  QTreeView::keyPressEvent(event);

  if (event->key() == Qt::Key_Delete) {
    deleteSelectedMessages();
  }
}

void MessagesView::contextMenuEvent(QContextMenuEvent *event) {
  QModelIndex clicked_index = indexAt(event->pos());

  if (!clicked_index.isValid()) {
    qDebug("Context menu for MessagesView will not be shown because user clicked on invalid item.");
    return;
  }

  if (m_contextMenu == NULL) {
    // Context menu is not initialized, initialize.
    initializeContextMenu();
  }

  m_contextMenu->exec(event->globalPos());
}

void MessagesView::initializeContextMenu() {
  m_contextMenu = new QMenu(tr("Context menu for messages"), this);
  m_contextMenu->addActions(QList<QAction*>() <<
                            qApp->mainForm()->m_ui->m_actionSendMessageViaEmail <<
                            qApp->mainForm()->m_ui->m_actionOpenSelectedSourceArticlesExternally <<
                            qApp->mainForm()->m_ui->m_actionOpenSelectedSourceArticlesInternally <<
                            qApp->mainForm()->m_ui->m_actionOpenSelectedMessagesInternally <<
                            qApp->mainForm()->m_ui->m_actionMarkSelectedMessagesAsRead <<
                            qApp->mainForm()->m_ui->m_actionMarkSelectedMessagesAsUnread <<
                            qApp->mainForm()->m_ui->m_actionSwitchImportanceOfSelectedMessages <<
                            qApp->mainForm()->m_ui->m_actionDeleteSelectedMessages);
}

void MessagesView::mousePressEvent(QMouseEvent *event) {
  QTreeView::mousePressEvent(event);

  switch (event->button()) {
    case Qt::LeftButton: {
      // Make sure that message importance is switched when user
      // clicks the "important" column.
      QModelIndex clicked_index = indexAt(event->pos());

      if (clicked_index.isValid()) {
        QModelIndex mapped_index = m_proxyModel->mapToSource(clicked_index);

        if (mapped_index.column() == MSG_DB_IMPORTANT_INDEX) {
          m_sourceModel->switchMessageImportance(mapped_index.row());
        }
      }

      break;
    }

    case Qt::MiddleButton: {
      // Open selected messages in new tab on mouse middle button click.
      openSelectedSourceMessagesInternally();
      break;
    }

    default:
      break;
  }
}

void MessagesView::currentChanged(const QModelIndex &current, const QModelIndex &previous) {
  QTreeView::currentChanged(current, previous);
}

void MessagesView::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) {
  QModelIndexList selected_rows = selectionModel()->selectedRows();
  QModelIndex current_index = currentIndex();
  QModelIndex mapped_current_index = m_proxyModel->mapToSource(current_index);

  qDebug("Current row changed - row [%d,%d] source [%d, %d].",
         current_index.row(), current_index.column(),
         mapped_current_index.row(), mapped_current_index.column());

  if (mapped_current_index.isValid() && selected_rows.count() == 1) {
    if (!m_batchUnreadSwitch) {
      // Set this message as read only if current item
      // wasn't changed by "mark selected messages unread" action.
      m_sourceModel->setMessageRead(mapped_current_index.row(), 1);
    }

    emit currentMessagesChanged(QList<Message>() << m_sourceModel->messageAt(m_proxyModel->mapToSource(selected_rows.at(0)).row()));
  }
  else {
    emit currentMessagesRemoved();
  }

  if (qApp->settings()->value(GROUP(Messages), SETTING(Messages::KeepCursorInCenter)).toBool()) {
    scrollTo(currentIndex(), QAbstractItemView::PositionAtCenter);
  }

  QTreeView::selectionChanged(selected, deselected);
}

void MessagesView::loadFeeds(RootItem *item) {
  m_sourceModel->loadMessages(item);

  int col = qApp->settings()->value(GROUP(GUI), SETTING(GUI::DefaultSortColumnMessages)).toInt();
  Qt::SortOrder ord = static_cast<Qt::SortOrder>(qApp->settings()->value(GROUP(GUI), SETTING(GUI::DefaultSortOrderMessages)).toInt());

  sortByColumn(col, ord);

#if QT_VERSION >= 0x050000
  // Messages are loaded, make sure that previously
  // active message is not shown in browser.
  // BUG: Qt 5 is probably bugged here. Selections
  // should be cleared automatically when SQL model is reset.
  emit currentMessagesRemoved();
#endif
}

void MessagesView::openSelectedSourceMessagesExternally() {
  foreach (const QModelIndex &index, selectionModel()->selectedRows()) {
    QString link = m_sourceModel->messageAt(m_proxyModel->mapToSource(index).row()).m_url;

    if (!WebFactory::instance()->openUrlInExternalBrowser(link)) {
      qApp->showGuiMessage(tr("Problem with starting external web browser"),
                           tr("External web browser could not be started."),
                           QSystemTrayIcon::Critical);
      return;
    }
  }

  // Finally, mark opened messages as read.
  if (selectionModel()->selectedRows().size() > 1) {
    markSelectedMessagesRead();
  }
}

void MessagesView::openSelectedSourceMessagesInternally() {
  foreach (const QModelIndex &index, selectionModel()->selectedRows()) {
    Message message = m_sourceModel->messageAt(m_proxyModel->mapToSource(index).row());

    if (message.m_url.isEmpty()) {
      MessageBox::show(this,
                       QMessageBox::Warning,
                       tr("Meesage without URL"),
                       tr("Message '%s' does not contain URL.").arg(message.m_title));
    }
    else {
      emit openLinkNewTab(message.m_url);
    }
  }

  // Finally, mark opened messages as read.
  if (selectionModel()->selectedRows().size() > 1) {
    markSelectedMessagesRead();
  }
}

void MessagesView::openSelectedSourceMessagesInternallyNoNewTab() {
  if (selectionModel()->selectedRows().size() == 1) {
    emit openLinkMiniBrowser(
          m_sourceModel->messageAt(m_proxyModel->mapToSource(selectionModel()->selectedRows().at(0)).row()).m_url);
  }
}

void MessagesView::openSelectedMessagesInternally() {
  QList<Message> messages;

  foreach (const QModelIndex &index, selectionModel()->selectedRows()) {
    messages << m_sourceModel->messageAt(m_proxyModel->mapToSource(index).row());
  }

  if (!messages.isEmpty()) {
    emit openMessagesInNewspaperView(messages);

    // Finally, mark opened messages as read.
    markSelectedMessagesRead();
  }
}

void MessagesView::sendSelectedMessageViaEmail() {
  if (selectionModel()->selectedRows().size() == 1) {
    Message message = m_sourceModel->messageAt(m_proxyModel->mapToSource(selectionModel()->selectedRows().at(0)).row());

    if (!WebFactory::instance()->sendMessageViaEmail(message)) {
      MessageBox::show(this,
                       QMessageBox::Critical,
                       tr("Problem with starting external e-mail client"),
                       tr("External e-mail client could not be started."));
    }
  }
}

void MessagesView::markSelectedMessagesRead() {
  setSelectedMessagesReadStatus(RootItem::Read);
}

void MessagesView::markSelectedMessagesUnread() {
  setSelectedMessagesReadStatus(RootItem::Unread);
}

void MessagesView::setSelectedMessagesReadStatus(RootItem::ReadStatus read) {
  QModelIndex current_index = selectionModel()->currentIndex();

  if (!current_index.isValid()) {
    return;
  }

  QModelIndex mapped_current_index = m_proxyModel->mapToSource(current_index);
  QModelIndexList selected_indexes = selectionModel()->selectedRows();
  QModelIndexList mapped_indexes = m_proxyModel->mapListToSource(selected_indexes);

  m_sourceModel->setBatchMessagesRead(mapped_indexes, read);
  sortByColumn(header()->sortIndicatorSection(), header()->sortIndicatorOrder());

  selected_indexes = m_proxyModel->mapListFromSource(mapped_indexes, true);
  current_index = m_proxyModel->mapFromSource(m_sourceModel->index(mapped_current_index.row(), mapped_current_index.column()));

  if (read == RootItem::Unread) {
    // User selected to mark some messages as unread, if one
    // of them will be marked as current, then it will be read again.
    m_batchUnreadSwitch = true;
  }

  setCurrentIndex(current_index);
  scrollTo(current_index);
  reselectIndexes(selected_indexes);
  m_batchUnreadSwitch = false;
}


// TODO: restore messages je uplně stejně jako tahle fce
// akorat se vola jina metoda na source modelu.
void MessagesView::deleteSelectedMessages() {
  QModelIndex current_index = selectionModel()->currentIndex();

  if (!current_index.isValid()) {
    return;
  }

  QModelIndexList selected_indexes = selectionModel()->selectedRows();
  QModelIndexList mapped_indexes = m_proxyModel->mapListToSource(selected_indexes);

  m_sourceModel->setBatchMessagesDeleted(mapped_indexes, 1);
  sortByColumn(header()->sortIndicatorSection(), header()->sortIndicatorOrder());

  int row_count = m_sourceModel->rowCount();
  if (row_count > 0) {
    QModelIndex last_item = current_index.row() < row_count ?
                              m_proxyModel->index(current_index.row(), MSG_DB_TITLE_INDEX) :
                              m_proxyModel->index(row_count - 1, MSG_DB_TITLE_INDEX);

    setCurrentIndex(last_item);
    scrollTo(last_item);
    reselectIndexes(QModelIndexList() << last_item);
  }
  else {
    emit currentMessagesRemoved();
  }
}

void MessagesView::switchSelectedMessagesImportance() {
  QModelIndex current_index = selectionModel()->currentIndex();

  if (!current_index.isValid()) {
    return;
  }

  QModelIndex mapped_current_index = m_proxyModel->mapToSource(current_index);
  QModelIndexList selected_indexes = selectionModel()->selectedRows();
  QModelIndexList mapped_indexes = m_proxyModel->mapListToSource(selected_indexes);

  m_sourceModel->switchBatchMessageImportance(mapped_indexes);
  sortByColumn(header()->sortIndicatorSection(), header()->sortIndicatorOrder());

  selected_indexes = m_proxyModel->mapListFromSource(mapped_indexes, true);
  current_index = m_proxyModel->mapFromSource(m_sourceModel->index(mapped_current_index.row(),
                                                                   mapped_current_index.column()));

  setCurrentIndex(current_index);
  scrollTo(current_index);
  reselectIndexes(selected_indexes);
}

void MessagesView::reselectIndexes(const QModelIndexList &indexes) {
  QItemSelection selection;

  foreach (const QModelIndex &index, indexes) {
    // TODO: THIS IS very slow. Try to select 4000 messages and hit "mark as read" button.
    selection.merge(QItemSelection(index, index), QItemSelectionModel::Select);
  }

  selectionModel()->select(selection, QItemSelectionModel::Select | QItemSelectionModel::Rows);
}

void MessagesView::selectNextItem() {
  QModelIndex index_next = moveCursor(QAbstractItemView::MoveDown, Qt::NoModifier);

  if (index_next.isValid()) {
    setCurrentIndex(index_next);
    selectionModel()->select(index_next, QItemSelectionModel::Select | QItemSelectionModel::Rows);
    setFocus();
  }
}

void MessagesView::selectPreviousItem() {
  QModelIndex index_previous = moveCursor(QAbstractItemView::MoveUp, Qt::NoModifier);

  if (index_previous.isValid()) {
    setCurrentIndex(index_previous);
    selectionModel()->select(index_previous, QItemSelectionModel::Select | QItemSelectionModel::Rows);
    setFocus();
  }
}

void MessagesView::searchMessages(const QString &pattern) {
  m_proxyModel->setFilterRegExp(pattern);

  if (selectionModel()->selectedRows().size() == 0) {
    emit currentMessagesRemoved();
  }
  else {
    // Scroll to selected message, it could become scrolled out due to filter change.
    scrollTo(selectionModel()->selectedRows().at(0));
  }
}

void MessagesView::filterMessages(MessagesModel::MessageHighlighter filter) {
  m_sourceModel->highlightMessages(filter);
}

void MessagesView::adjustColumns() {
  if (header()->count() > 0 && !m_columnsAdjusted) {
    m_columnsAdjusted = true;

#if QT_VERSION >= 0x050000
    // Setup column resize strategies.
    header()->setSectionResizeMode(MSG_DB_ID_INDEX, QHeaderView::Interactive);
    header()->setSectionResizeMode(MSG_DB_READ_INDEX, QHeaderView::ResizeToContents);
    header()->setSectionResizeMode(MSG_DB_DELETED_INDEX, QHeaderView::Interactive);
    header()->setSectionResizeMode(MSG_DB_IMPORTANT_INDEX, QHeaderView::ResizeToContents);
    header()->setSectionResizeMode(MSG_DB_FEED_INDEX, QHeaderView::Interactive);
    header()->setSectionResizeMode(MSG_DB_TITLE_INDEX, QHeaderView::Stretch);
    header()->setSectionResizeMode(MSG_DB_URL_INDEX, QHeaderView::Interactive);
    header()->setSectionResizeMode(MSG_DB_AUTHOR_INDEX, QHeaderView::Interactive);
    header()->setSectionResizeMode(MSG_DB_DCREATED_INDEX, QHeaderView::Interactive);
    header()->setSectionResizeMode(MSG_DB_CONTENTS_INDEX, QHeaderView::Interactive);
    header()->setSectionResizeMode(MSG_DB_PDELETED_INDEX, QHeaderView::Interactive);
#else
    // Setup column resize strategies.
    header()->setResizeMode(MSG_DB_ID_INDEX, QHeaderView::Interactive);
    header()->setResizeMode(MSG_DB_READ_INDEX, QHeaderView::ResizeToContents);
    header()->setResizeMode(MSG_DB_DELETED_INDEX, QHeaderView::Interactive);
    header()->setResizeMode(MSG_DB_IMPORTANT_INDEX, QHeaderView::ResizeToContents);
    header()->setResizeMode(MSG_DB_FEED_INDEX, QHeaderView::Interactive);
    header()->setResizeMode(MSG_DB_TITLE_INDEX, QHeaderView::Stretch);
    header()->setResizeMode(MSG_DB_URL_INDEX, QHeaderView::Interactive);
    header()->setResizeMode(MSG_DB_AUTHOR_INDEX, QHeaderView::Interactive);
    header()->setResizeMode(MSG_DB_DCREATED_INDEX, QHeaderView::Interactive);
    header()->setResizeMode(MSG_DB_CONTENTS_INDEX, QHeaderView::Interactive);
    header()->setResizeMode(MSG_DB_PDELETED_INDEX, QHeaderView::Interactive);
#endif

    // Hide columns.
    hideColumn(MSG_DB_ID_INDEX);
    hideColumn(MSG_DB_DELETED_INDEX);
    hideColumn(MSG_DB_FEED_INDEX);
    hideColumn(MSG_DB_URL_INDEX);
    hideColumn(MSG_DB_CONTENTS_INDEX);
    hideColumn(MSG_DB_PDELETED_INDEX);
    hideColumn(MSG_DB_ENCLOSURES_INDEX);

    qDebug("Adjusting column resize modes for MessagesView.");
  }
}

void MessagesView::saveSortState(int column, Qt::SortOrder order) {
  qApp->settings()->setValue(GROUP(GUI), GUI::DefaultSortColumnMessages, column);
  qApp->settings()->setValue(GROUP(GUI), GUI::DefaultSortOrderMessages, order);
}
