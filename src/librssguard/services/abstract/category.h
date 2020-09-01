// For license of this file, see <project-root-folder>/LICENSE.md.

#ifndef CATEGORY_H
#define CATEGORY_H

#include "services/abstract/rootitem.h"

class Category : public RootItem
{
    Q_OBJECT

public:
    explicit Category(RootItem *parent = nullptr);
    explicit Category(const Category &other);
    explicit Category(const QSqlRecord &record);
    virtual ~Category();

    void updateCounts(bool including_total_count);
    bool cleanMessages(bool clean_read_only);
    bool markAsReadUnread(ReadStatus status);
};

#endif // CATEGORY_H
