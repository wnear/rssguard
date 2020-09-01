// For license of this file, see <project-root-folder>/LICENSE.md.

#include "dynamic-shortcuts/dynamicshortcutswidget.h"

#include "definitions/definitions.h"
#include "dynamic-shortcuts/shortcutcatcher.h"

#include <QAction>
#include <QGridLayout>
#include <QLabel>

DynamicShortcutsWidget::DynamicShortcutsWidget(QWidget *parent) : QWidget(parent)
{
    // Create layout for this control and set is as active.
    m_layout = new QGridLayout(this);
    m_layout->setMargin(0);
    setLayout(m_layout);
}

DynamicShortcutsWidget::~DynamicShortcutsWidget()
{
    delete m_layout;
}

bool DynamicShortcutsWidget::areShortcutsUnique() const
{
    QList<QKeySequence> all_shortcuts;

    // Obtain all shortcuts.
    for (const ActionBinding &binding : m_actionBindings) {
        const QKeySequence new_shortcut = binding.second->shortcut();

        if (!new_shortcut.isEmpty() && all_shortcuts.contains(new_shortcut)) {
            // Problem, two identical non-empty shortcuts found.
            return false;
        } else {
            all_shortcuts.append(binding.second->shortcut());
        }
    }

    return true;
}

void DynamicShortcutsWidget::updateShortcuts()
{
    for (const ActionBinding &binding : m_actionBindings) {
        binding.first->setShortcut(binding.second->shortcut());
    }
}

void DynamicShortcutsWidget::populate(QList<QAction *> actions)
{
    m_actionBindings.clear();
    std::sort(actions.begin(), actions.end(), [](QAction * lhs, QAction * rhs) {
        return QString::localeAwareCompare(lhs->text().replace(QL1S("&"), QString()),
                                           rhs->text().replace(QL1S("&"), QString())) < 0;
    });
    int row_id = 0;

    // FIXME: Maybe separate actions into "categories". Each category will start with label.
    // I will assign each QAaction a property called "category" with some enum value.
    // Like:
    //  File, FeedsCategories, Messages, Tools, WebBrowser, Help
    // This will be setup in FormMain::allActions().
    // Then here I will process actions into categories.

    for (QAction *action : actions) {
        // Create shortcut catcher for this action and set default shortcut.
        auto *catcher = new ShortcutCatcher(this);

        catcher->setDefaultShortcut(action->shortcut());

        // Store information for re-initialization of shortcuts
        // of actions when widget gets "confirmed".
        QPair<QAction *, ShortcutCatcher *> new_binding;

        new_binding.first = action;
        new_binding.second = catcher;
        m_actionBindings << new_binding;

        // Add new catcher to our control.
        auto *action_label = new QLabel(this);

        action_label->setText(action->text().remove(QSL("&")));
        action_label->setToolTip(action->toolTip());
        action_label->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);

        auto *action_icon = new QLabel(this);

        action_icon->setPixmap(action->icon().pixmap(ICON_SIZE_SETTINGS, ICON_SIZE_SETTINGS));
        action_icon->setToolTip(action->toolTip());
        m_layout->addWidget(action_icon, row_id, 0);
        m_layout->addWidget(action_label, row_id, 1);
        m_layout->addWidget(catcher, row_id, 2);
        row_id++;
        connect(catcher, &ShortcutCatcher::shortcutChanged, this, &DynamicShortcutsWidget::setupChanged);
    }

    // Make sure that "spacer" is added.
    m_layout->setRowStretch(row_id, 1);
    m_layout->setColumnStretch(1, 1);
}
