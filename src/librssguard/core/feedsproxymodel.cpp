// For license of this file, see <project-root-folder>/LICENSE.md.

#include "core/feedsproxymodel.h"

#include "core/feedsmodel.h"
#include "definitions/definitions.h"
#include "miscellaneous/application.h"
#include "miscellaneous/regexfactory.h"
#include "services/abstract/rootitem.h"

#include <QTimer>

FeedsProxyModel::FeedsProxyModel(FeedsModel *source_model, QObject *parent)
    : QSortFilterProxyModel(parent), m_sourceModel(source_model), m_selectedItem(nullptr),
      m_showUnreadOnly(false)
{
    setObjectName(QSL("FeedsProxyModel"));
    setSortRole(Qt::EditRole);
    setSortCaseSensitivity(Qt::CaseInsensitive);
    setFilterCaseSensitivity(Qt::CaseInsensitive);
    setFilterKeyColumn(-1);
    setFilterRole(Qt::EditRole);
    setDynamicSortFilter(true);
    setSourceModel(m_sourceModel);

    // Describes priorities of node types for sorting.
    // Smaller index means that item is "smaller" which
    // means it should be more on top when sorting
    // in ascending order.
    m_priorities = {
        RootItem::Kind::Category,
        RootItem::Kind::Feed,
        RootItem::Kind::Labels,
        RootItem::Kind::Important,
        RootItem::Kind::Bin
    };
}

FeedsProxyModel::~FeedsProxyModel()
{
    qDebugNN << LOGSEC_FEEDMODEL << "Destroying FeedsProxyModel instance";
}

QModelIndexList FeedsProxyModel::match(const QModelIndex &start, int role, const QVariant &value,
                                       int hits, Qt::MatchFlags flags) const
{
    QModelIndexList result;
    const int match_type = flags & 0x0F;
    const Qt::CaseSensitivity cs = Qt::CaseInsensitive;
    const bool recurse = (flags & Qt::MatchRecursive) > 0;
    const bool wrap = (flags & Qt::MatchWrap) > 0;
    const bool all_hits = (hits == -1);
    QString entered_text;
    const QModelIndex p = parent(start);
    int from = start.row();
    int to = rowCount(p);

    for (int i = 0; (wrap && i < 2) || (!wrap && i < 1); ++i) {
        for (int r = from; (r < to) && (all_hits || result.count() < hits); ++r) {
            QModelIndex idx = index(r, start.column(), p);

            if (!idx.isValid()) {
                continue;
            }

            QModelIndex mapped_idx = mapToSource(idx);
            QVariant item_value = m_sourceModel->itemForIndex(mapped_idx)->title();

            // QVariant based matching.
            if (match_type == Qt::MatchFlag::MatchExactly) {
                if (value == item_value) {
                    result.append(idx);
                }
            }

            // QString based matching.
            else {
                if (entered_text.isEmpty()) {
                    entered_text = value.toString();
                }

                QString item_text = item_value.toString();

                switch (match_type) {
#if QT_VERSION >= 0x050F00 // Qt >= 5.15.0
                    case Qt::MatchFlag::MatchRegularExpression:
#else
                    case Qt::MatchFlag::MatchRegExp:
#endif
                        if (QRegularExpression(entered_text,
                                               QRegularExpression::PatternOption::CaseInsensitiveOption |
                                               QRegularExpression::PatternOption::UseUnicodePropertiesOption).match(item_text).hasMatch()) {
                            result.append(idx);
                        }

                        break;

                    case Qt::MatchFlag::MatchWildcard:
                        if (QRegularExpression(RegexFactory::wildcardToRegularExpression(entered_text),
                                               QRegularExpression::PatternOption::CaseInsensitiveOption |
                                               QRegularExpression::PatternOption::UseUnicodePropertiesOption).match(item_text).hasMatch()) {
                            result.append(idx);
                        }

                        break;

                    case Qt::MatchFlag::MatchStartsWith:
                        if (item_text.startsWith(entered_text, cs)) {
                            result.append(idx);
                        }

                        break;

                    case Qt::MatchFlag::MatchEndsWith:
                        if (item_text.endsWith(entered_text, cs)) {
                            result.append(idx);
                        }

                        break;

                    case Qt::MatchFlag::MatchFixedString:
                        if (item_text.compare(entered_text, cs) == 0) {
                            result.append(idx);
                        }

                        break;

                    case Qt::MatchFlag::MatchContains:
                    default:
                        if (item_text.contains(entered_text, cs)) {
                            result.append(idx);
                        }

                        break;
                }
            }

            if (recurse && hasChildren(idx)) {
                result +=
                    match(index(0, idx.column(), idx), role, (entered_text.isEmpty() ? value : entered_text),
                          (all_hits ? -1 : hits - result.count()), flags);
            }
        }

        from = 0;
        to = start.row();
    }

    return result;
}

bool FeedsProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    if (left.isValid() && right.isValid()) {
        // Make necessary castings.
        const RootItem *left_item = m_sourceModel->itemForIndex(left);
        const RootItem *right_item = m_sourceModel->itemForIndex(right);

        // NOTE: Here we want to accomplish that ALL
        // categories are queued one after another and all
        // feeds are queued one after another too.
        // Moreover, sort everything alphabetically or
        // by item counts, depending on the sort column.

        if (left_item->keepOnTop()) {
            return sortOrder() == Qt::SortOrder::AscendingOrder;
        } else if (right_item->keepOnTop()) {
            return sortOrder() == Qt::SortOrder::DescendingOrder;
        } else if (left_item->kind() == right_item->kind()) {
            // Both items are of the same type.
            if (left.column() == FDS_MODEL_COUNTS_INDEX) {
                // User wants to sort according to counts.
                return left_item->countOfUnreadMessages() < right_item->countOfUnreadMessages();
            } else {
                // In other cases, sort by title.
                return QString::localeAwareCompare(left_item->title().toLower(), right_item->title().toLower()) < 0;
            }
        } else {
            // We sort using priorities.
            auto left_priority = m_priorities.indexOf(left_item->kind());
            auto right_priority = m_priorities.indexOf(right_item->kind());

            return sortOrder() == Qt::SortOrder::AscendingOrder
                   ? left_priority < right_priority
                   : right_priority < left_priority;
        }
    } else {
        return false;
    }
}

bool FeedsProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    bool should_show = filterAcceptsRowInternal(source_row, source_parent);

    if (should_show && m_hiddenIndices.contains(QPair<int, QModelIndex>(source_row, source_parent))) {
        const_cast<FeedsProxyModel *>(this)->m_hiddenIndices.removeAll(QPair<int, QModelIndex>(source_row,
                source_parent));

        // Load status.
        emit expandAfterFilterIn(m_sourceModel->index(source_row, 0, source_parent));
    }

    if (!should_show) {
        const_cast<FeedsProxyModel *>(this)->m_hiddenIndices.append(QPair<int, QModelIndex>(source_row,
                source_parent));
    }

    return should_show;
}

bool FeedsProxyModel::filterAcceptsRowInternal(int source_row,
        const QModelIndex &source_parent) const
{
    if (!m_showUnreadOnly) {
        return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
    }

    const QModelIndex idx = m_sourceModel->index(source_row, 0, source_parent);

    if (!idx.isValid()) {
        return false;
    }

    const RootItem *item = m_sourceModel->itemForIndex(idx);

    if (item->kind() != RootItem::Kind::Category && item->kind() != RootItem::Kind::Feed) {
        // Some items are always visible.
        return true;
    } else if (item->isParentOf(m_selectedItem) /* || item->isChildOf(m_selectedItem)*/
               || m_selectedItem == item) {
        // Currently selected item and all its parents and children must be displayed.
        return true;
    } else {
        // NOTE: If item has < 0 of unread message it may mean, that the count
        // of unread messages is not (yet) known, display that item too.
        return item->countOfUnreadMessages() != 0;
    }
}

const RootItem *FeedsProxyModel::selectedItem() const
{
    return m_selectedItem;
}

void FeedsProxyModel::setSelectedItem(const RootItem *selected_item)
{
    m_selectedItem = selected_item;
}

bool FeedsProxyModel::showUnreadOnly() const
{
    return m_showUnreadOnly;
}

void FeedsProxyModel::invalidateReadFeedsFilter(bool set_new_value, bool show_unread_only)
{
    if (set_new_value) {
        setShowUnreadOnly(show_unread_only);
    }

    QTimer::singleShot(0, this, &FeedsProxyModel::invalidateFilter);
}

void FeedsProxyModel::setShowUnreadOnly(bool show_unread_only)
{
    m_showUnreadOnly = show_unread_only;
    qApp->settings()->setValue(GROUP(Feeds), Feeds::ShowOnlyUnreadFeeds, show_unread_only);
}

QModelIndexList FeedsProxyModel::mapListToSource(const QModelIndexList &indexes) const
{
    QModelIndexList source_indexes;

    for (const QModelIndex &index : indexes) {
        source_indexes << mapToSource(index);
    }

    return source_indexes;
}
