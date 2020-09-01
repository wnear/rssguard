// For license of this file, see <project-root-folder>/LICENSE.md.

#include "gui/searchtextwidget.h"

#include "miscellaneous/application.h"
#include "miscellaneous/iconfactory.h"

#include <QKeyEvent>

SearchTextWidget::SearchTextWidget(QWidget *parent) : QWidget(parent)
{
    m_ui.setupUi(this);
    setFocusProxy(m_ui.m_txtSearch);

    m_ui.m_btnClear->setIcon(qApp->icons()->fromTheme(QSL("edit-clear")));
    m_ui.m_btnSearchBackward->setIcon(qApp->icons()->fromTheme(QSL("back")));
    m_ui.m_btnSearchForward->setIcon(qApp->icons()->fromTheme(QSL("forward")));

    connect(m_ui.m_btnClear, &QToolButton::clicked, m_ui.m_txtSearch, &QLineEdit::clear);
    connect(m_ui.m_txtSearch, &BaseLineEdit::textChanged, this, &SearchTextWidget::onTextChanged);
    connect(m_ui.m_txtSearch, &BaseLineEdit::submitted, this, [this]() {
        emit searchForText(m_ui.m_txtSearch->text(), false);
    });
    connect(m_ui.m_btnSearchForward, &QToolButton::clicked, this, [this]() {
        emit searchForText(m_ui.m_txtSearch->text(), false);
    });
    connect(m_ui.m_btnSearchBackward, &QToolButton::clicked, this, [this]() {
        emit searchForText(m_ui.m_txtSearch->text(), true);
    });
}

void SearchTextWidget::clear()
{
    m_ui.m_txtSearch->clear();
}

void SearchTextWidget::onTextChanged(const QString &text)
{
    m_ui.m_btnSearchBackward->setDisabled(text.isEmpty());
    m_ui.m_btnSearchForward->setDisabled(text.isEmpty());
    m_ui.m_btnClear->setDisabled(text.isEmpty());

    if (!text.isEmpty()) {
        emit searchForText(text, false);
    } else {
        emit cancelSearch();
    }
}

void SearchTextWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key::Key_Escape) {
        emit cancelSearch();

        hide();
    }
}

void SearchTextWidget::focusInEvent(QFocusEvent *event)
{
    QWidget::focusInEvent(event);
}
