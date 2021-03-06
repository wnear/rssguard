// For license of this file, see <project-root-folder>/LICENSE.md.

#ifndef MESSAGEFILTER_H
#define MESSAGEFILTER_H

#include <QObject>

#include "core/message.h"

class QJSEngine;

// Class which represents one message filter.
class MessageFilter : public QObject
{
    Q_OBJECT

public:
    explicit MessageFilter(int id = -1, QObject *parent = nullptr);

    FilteringAction filterMessage(QJSEngine *engine);

    int id() const;
    void setId(int id);

    QString name() const;
    void setName(const QString &name);

    QString script() const;
    void setScript(const QString &script);

private:
    int m_id;
    QString m_name;
    QString m_script;
};

#endif // MESSAGEFILTER_H
