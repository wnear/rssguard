// For license of this file, see <project-root-folder>/LICENSE.md.

#ifndef SHORTCUTCATCHER_H
#define SHORTCUTCATCHER_H

#include <QWidget>

class QHBoxLayout;
class PlainToolButton;
class QKeySequenceEdit;

class ShortcutCatcher : public QWidget
{
    Q_OBJECT

public:
    explicit ShortcutCatcher(QWidget *parent = nullptr);

    QKeySequence shortcut() const;
    void setDefaultShortcut(const QKeySequence &key);
    void setShortcut(const QKeySequence &key);

public slots:
    void resetShortcut();
    void clearShortcut();

signals:
    void shortcutChanged(const QKeySequence &seguence);

private:
    PlainToolButton *m_btnReset;
    PlainToolButton *m_btnClear;
    QKeySequenceEdit *m_shortcutBox;
    QHBoxLayout *m_layout;
    QKeySequence m_currentSequence;
    QKeySequence m_defaultSequence;
    bool m_isRecording;
    int m_numKey;
    int m_modifierKeys;
};

#endif // KEYSEQUENCECATCHER_H
