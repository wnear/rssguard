// For license of this file, see <project-root-folder>/LICENSE.md.

#include "core/messagesproxymodel.h"

#include "core/messagesmodel.h"
#include "core/messagesmodelcache.h"
#include "miscellaneous/application.h"
#include "miscellaneous/regexfactory.h"
#include "miscellaneous/settings.h"

#include <QTimer>

MessagesProxyModel::MessagesProxyModel(MessagesModel *source_model, QObject *parent)
    : QSortFilterProxyModel(parent), m_sourceModel(source_model), m_showUnreadOnly(false)
{
    setObjectName(QSL("MessagesProxyModel"));

    setSortRole(Qt::EditRole);
    setSortCaseSensitivity(Qt::CaseInsensitive);

    setFilterCaseSensitivity(Qt::CaseInsensitive);
    setFilterKeyColumn(-1);
    setFilterRole(Qt::EditRole);

    setDynamicSortFilter(false);
    setSourceModel(m_sourceModel);
}

MessagesProxyModel::~MessagesProxyModel()
{
    qDebugNN << LOGSEC_MESSAGEMODEL << "Destroying MessagesProxyModel instance.";
}

QModelIndex MessagesProxyModel::getNextPreviousUnreadItemIndex(int default_row)
{
    const bool started_from_zero = default_row == 0;
    QModelIndex next_index = getNextUnreadItemIndex(default_row, rowCount() - 1);

    // There is no next message, check previous.
    if (!next_index.isValid() && !started_from_zero) {
        next_index = getNextUnreadItemIndex(0, default_row - 1);
    }

    return next_index;
}

QModelIndex MessagesProxyModel::getNextUnreadItemIndex(int default_row, int max_row) const
{
    while (default_row <= max_row) {
        // Get info if the message is read or not.
        const QModelIndex proxy_index = index(default_row, MSG_DB_READ_INDEX);
        const bool is_read = m_sourceModel->data(mapToSource(proxy_index).row(),
                             MSG_DB_READ_INDEX, Qt::EditRole).toInt() == 1;

        if (!is_read) {
            // We found unread message, mark it.
            return proxy_index;
        } else {
            default_row++;
        }
    }

    return QModelIndex();
}

bool MessagesProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    Q_UNUSED(left)
    Q_UNUSED(right)

    // NOTE: Comparisons are done by SQL servers itself, not client-side.
    return false;
}

bool MessagesProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    // We want to show only regexped messages when "all" should be visible
    // and we want to show only regexped AND unread messages when unread should be visible.
    //
    // But also, we want to see messages which have their dirty states cached, because
    // otherwise they would just disappeaar from the list for example when batch marked as read
    // which is distracting.
    return
        QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent) &&
        (m_sourceModel->cache()->containsData(source_row) ||
         (!m_showUnreadOnly || !m_sourceModel->messageAt(source_row).m_isRead));
}

bool MessagesProxyModel::showUnreadOnly() const
{
    return m_showUnreadOnly;
}

void MessagesProxyModel::setShowUnreadOnly(bool show_unread_only)
{
    m_showUnreadOnly = show_unread_only;
    qApp->settings()->setValue(GROUP(Messages), Messages::ShowOnlyUnreadMessages, show_unread_only);
}

QModelIndexList MessagesProxyModel::mapListFromSource(const QModelIndexList &indexes,
        bool deep) const
{
    QModelIndexList mapped_indexes;

    for (const QModelIndex &index : indexes) {
        if (deep) {
            // Construct new source index.
            mapped_indexes << mapFromSource(m_sourceModel->index(index.row(), index.column()));
        } else {
            mapped_indexes << mapFromSource(index);
        }
    }

    return mapped_indexes;
}

QModelIndexList MessagesProxyModel::match(const QModelIndex &start, int role,
        const QVariant &entered_value, int hits, Qt::MatchFlags flags) const
{
    QModelIndexList result;
    const int match_type = flags & 0x0F;
    const Qt::CaseSensitivity case_sensitivity = Qt::CaseInsensitive;
    const bool wrap = (flags & Qt::MatchWrap) > 0;
    const bool all_hits = (hits == -1);
    QString entered_text;
    int from = start.row();
    int to = rowCount();

    for (int i = 0; (wrap && i < 2) || (!wrap && i < 1); i++) {
        for (int r = from; (r < to) && (all_hits || result.count() < hits); r++) {
            QModelIndex idx = index(r, start.column());

            if (!idx.isValid()) {
                continue;
            }

            QVariant item_value = m_sourceModel->data(mapToSource(idx).row(), MSG_DB_TITLE_INDEX, role);

            // QVariant based matching.
            if (match_type == Qt::MatchExactly) {
                if (entered_value == item_value) {
                    result.append(idx);
                }
            }

            // QString based matching.
            else {
                if (entered_text.isEmpty()) {
                    entered_text = entered_value.toString();
                }

                QString item_text = item_value.toString();

                switch (match_type) {
#if QT_VERSION >= 0x050F00 // Qt >= 5.15.0
                    case Qt::MatchRegularExpression:
#else
                    case Qt::MatchRegExp:
#endif
                        if (QRegularExpression(entered_text,
                                               QRegularExpression::PatternOption::CaseInsensitiveOption |
                                               QRegularExpression::PatternOption::UseUnicodePropertiesOption).match(item_text).hasMatch()) {
                            result.append(idx);
                        }

                        break;

                    case Qt::MatchWildcard:
                        if (QRegularExpression(RegexFactory::wildcardToRegularExpression(entered_text),
                                               QRegularExpression::PatternOption::CaseInsensitiveOption |
                                               QRegularExpression::PatternOption::UseUnicodePropertiesOption).match(item_text).hasMatch()) {
                            result.append(idx);
                        }

                        break;

                    case Qt::MatchStartsWith:
                        if (item_text.startsWith(entered_text, case_sensitivity)) {
                            result.append(idx);
                        }

                        break;

                    case Qt::MatchEndsWith:
                        if (item_text.endsWith(entered_text, case_sensitivity)) {
                            result.append(idx);
                        }

                        break;

                    case Qt::MatchFixedString:
                        if (item_text.compare(entered_text, case_sensitivity) == 0) {
                            result.append(idx);
                        }

                        break;

                    case Qt::MatchContains:
                    default:
                        if (item_text.contains(entered_text, case_sensitivity)) {
                            result.append(idx);
                        }

                        break;
                }
            }
        }

        // Prepare for the next iteration.
        from = 0;
        to = start.row();
    }

    return result;
}

void MessagesProxyModel::sort(int column, Qt::SortOrder order)
{
    // NOTE: Ignore here, sort is done elsewhere (server-side).
    Q_UNUSED(column)
    Q_UNUSED(order)
}

QModelIndexList MessagesProxyModel::mapListToSource(const QModelIndexList &indexes) const
{
    QModelIndexList source_indexes;

    for (const QModelIndex &index : indexes) {
        source_indexes << mapToSource(index);
    }

    return source_indexes;
}
