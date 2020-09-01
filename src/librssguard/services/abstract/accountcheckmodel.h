// For license of this file, see <project-root-folder>/LICENSE.md.

#ifndef ACCOUNTCHECKMODEL_H
#define ACCOUNTCHECKMODEL_H

#include <QAbstractItemModel>

#include "services/abstract/rootitem.h"

// This is common model which displays only categories/feeds
// and allows user to place checkmarks.
class AccountCheckModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit AccountCheckModel(QObject *parent = nullptr);
    virtual ~AccountCheckModel();

    QModelIndex index(int row, int column, const QModelIndex &parent) const;
    QModelIndex parent(const QModelIndex &child) const;
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    Qt::ItemFlags flags(const QModelIndex &index) const;

    bool isItemChecked(RootItem *item);
    bool setItemChecked(RootItem *item, Qt::CheckState check);

    // Returns feed/category which lies at the specified index or
    // root item if index is invalid.
    RootItem *itemForIndex(const QModelIndex &index) const;

    // Returns source QModelIndex on which lies given item.
    QModelIndex indexForItem(RootItem *item) const;

    // Root item manipulators.
    RootItem *rootItem() const;

    void setRootItem(RootItem *root_item, bool delete_previous_root = true,
                     bool with_layout_change = false);

public slots:
    void checkAllItems();
    void uncheckAllItems();

signals:
    void checkStateChanged(RootItem *item, Qt::CheckState state);

protected:
    RootItem *m_rootItem;

private:
    QHash<RootItem *, Qt::CheckState> m_checkStates;
    bool m_recursiveChange;
};

#endif // ACCOUNTCHECKMODEL_H
