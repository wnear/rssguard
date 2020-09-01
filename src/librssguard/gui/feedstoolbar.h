// For license of this file, see <project-root-folder>/LICENSE.md.

#ifndef FEEDSTOOLBAR_H
#define FEEDSTOOLBAR_H

#include "gui/basetoolbar.h"

class FeedsToolBar : public BaseToolBar
{
    Q_OBJECT

public:
    explicit FeedsToolBar(const QString &title, QWidget *parent = nullptr);

    QList<QAction *> availableActions() const;
    QList<QAction *> changeableActions() const;
    void saveChangeableActions(const QStringList &actions);

    QList<QAction *> getSpecificActions(const QStringList &actions);
    void loadSpecificActions(const QList<QAction *> &actions, bool initial_load = false);

    QStringList defaultActions() const;
    QStringList savedActions() const;
};

#endif // FEEDSTOOLBAR_H
