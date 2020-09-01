// For license of this file, see <project-root-folder>/LICENSE.md.

#ifndef NEWSPAPERPREVIEWER_H
#define NEWSPAPERPREVIEWER_H

#include <QWidget>

#include "gui/tabcontent.h"

#include "ui_newspaperpreviewer.h"

#include "core/message.h"
#include "services/abstract/rootitem.h"

#include <QPointer>

namespace Ui
{
class NewspaperPreviewer;
}

class RootItem;

class NewspaperPreviewer : public TabContent
{
    Q_OBJECT

public:
    explicit NewspaperPreviewer(RootItem *root, QList<Message> messages, QWidget *parent = nullptr);

private slots:
    void showMoreMessages();

signals:
    void markMessageRead(int id, RootItem::ReadStatus read);
    void markMessageImportant(int id, RootItem::Importance important);

private:
    QScopedPointer<Ui::NewspaperPreviewer> m_ui;
    QPointer<RootItem> m_root;
    QList<Message> m_messages;
};

#endif // NEWSPAPERPREVIEWER_H
