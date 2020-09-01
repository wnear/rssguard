// For license of this file, see <project-root-folder>/LICENSE.md.

#ifndef FEEDSVIEW_H
#define FEEDSVIEW_H

#include <QTreeView>

#include "core/feedsmodel.h"

#include <QStyledItemDelegate>

class FeedsProxyModel;
class Feed;
class Category;

class RSSGUARD_DLLSPEC FeedsView : public QTreeView
{
    Q_OBJECT

public:
    explicit FeedsView(QWidget *parent = nullptr);
    virtual ~FeedsView();

    FeedsProxyModel *model() const;
    FeedsModel *sourceModel() const;

    void reloadFontSettings();

    void setSortingEnabled(bool enable);

    // Returns list of selected/all feeds.
    // NOTE: This is recursive method which returns all descendants.
    QList<Feed *> selectedFeeds() const;

    // Returns pointers to selected feed/category if they are really
    // selected.
    RootItem *selectedItem() const;

    // Saves/loads expand states of all nodes (feeds/categories) of the list to/from settings.
    void saveAllExpandStates();
    void loadAllExpandStates();

public slots:
    void copyUrlOfSelectedFeeds() const;
    void sortByColumn(int column, Qt::SortOrder order);

    void addFeedIntoSelectedAccount();
    void addCategoryIntoSelectedAccount();
    void expandCollapseCurrentItem();

    // Feed updating.
    void updateSelectedItems();

    // Feed read/unread manipulators.
    void markSelectedItemRead();
    void markSelectedItemUnread();
    void markAllItemsRead();

    // Newspaper accessors.
    void openSelectedItemsInNewspaperMode();

    // Feed clearers.
    void clearSelectedFeeds();
    void clearAllFeeds();

    // Base manipulators.
    void editSelectedItem();
    void deleteSelectedItem();

    // Selects next/previous item (feed/category) in the list.
    void selectNextItem();
    void selectPreviousItem();

    void selectNextUnreadItem();

    // Switches visibility of the widget.
    void switchVisibility();

signals:
    void itemSelected(RootItem *item);
    void requestViewNextUnreadMessage();
    void openMessagesInNewspaperView(RootItem *root, const QList<Message> &messages);

protected:
    void drawBranches(QPainter *painter, const QRect &rect, const QModelIndex &index) const;
    void focusInEvent(QFocusEvent *event);
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void keyPressEvent(QKeyEvent *event);
    void contextMenuEvent(QContextMenuEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);

private slots:
    void expandItemDelayed(const QModelIndex &idx);
    void markSelectedItemReadStatus(RootItem::ReadStatus read);
    void markAllItemsReadStatus(RootItem::ReadStatus read);

    void saveSortState(int column, Qt::SortOrder order);
    void validateItemAfterDragDrop(const QModelIndex &source_index);
    void onItemExpandRequested(const QList<RootItem *> &items, bool exp);
    void onItemExpandStateSaveRequested(RootItem *item);

private:
    QModelIndex nextPreviousUnreadItem(QModelIndex default_row);
    QModelIndex nextUnreadItem(QModelIndex default_row);

    // Initializes context menus.
    QMenu *initializeContextMenuBin(RootItem *clicked_item);
    QMenu *initializeContextMenuService(RootItem *clicked_item);
    QMenu *initializeContextMenuCategories(RootItem *clicked_item);
    QMenu *initializeContextMenuFeeds(RootItem *clicked_item);
    QMenu *initializeContextMenuImportant(RootItem *clicked_item);
    QMenu *initializeContextMenuEmptySpace();
    QMenu *initializeContextMenuOtherItem(RootItem *clicked_item);

    void setupAppearance();
    void saveExpandStates(RootItem *item);

    QMenu *m_contextMenuService;
    QMenu *m_contextMenuBin;
    QMenu *m_contextMenuCategories;
    QMenu *m_contextMenuFeeds;
    QMenu *m_contextMenuImportant;
    QMenu *m_contextMenuEmptySpace;
    QMenu *m_contextMenuOtherItems;
    FeedsModel *m_sourceModel;
    FeedsProxyModel *m_proxyModel;
};

inline FeedsProxyModel *FeedsView::model() const
{
    return m_proxyModel;
}

inline FeedsModel *FeedsView::sourceModel() const
{
    return m_sourceModel;
}

#endif // FEEDSVIEW_H
