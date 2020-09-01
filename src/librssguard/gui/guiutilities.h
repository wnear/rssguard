// For license of this file, see <project-root-folder>/LICENSE.md.

#ifndef GUIUTILITIES_H
#define GUIUTILITIES_H

#include <QIcon>
#include <QLabel>
#include <QWidget>

class GuiUtilities
{
public:
    static void setLabelAsNotice(QLabel &label, bool is_warning);
    static void applyDialogProperties(QWidget &widget, const QIcon &icon = QIcon(),
                                      const QString &title = QString());
    static void applyResponsiveDialogResize(QWidget &widget, double factor = 0.6);

private:
    explicit GuiUtilities();
};

inline GuiUtilities::GuiUtilities() {}

#endif // GUIUTILITIES_H
