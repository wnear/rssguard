// For license of this file, see <project-root-folder>/LICENSE.md.

#ifndef ICONFACTORY_H
#define ICONFACTORY_H

#include <QObject>

#include "definitions/definitions.h"
#include "miscellaneous/application.h"

#include <QDir>
#include <QHash>
#include <QIcon>
#include <QString>

class RSSGUARD_DLLSPEC IconFactory : public QObject
{
    Q_OBJECT

public:
    explicit IconFactory(QObject *parent = nullptr);
    virtual ~IconFactory();

    // Used to store/retrieve QIcons from/to Base64-encoded
    // byte array.
    static QIcon fromByteArray(QByteArray array);
    static QByteArray toByteArray(const QIcon &icon);

    // Returns icon from active theme or invalid icon if
    // "no icon theme" is set.
    QIcon fromTheme(const QString &name);

    QPixmap miscPixmap(const QString &name);
    QIcon miscIcon(const QString &name);

    // Adds custom application path to be search for icons.
    void setupSearchPaths();

    // Returns list of installed themes, including "default" theme.
    QStringList installedIconThemes() const;

    // Loads name of selected icon theme (from settings) for the application and
    // activates it. If that particular theme is not installed, then
    // "default" theme is loaded.
    void loadCurrentIconTheme();

    // Returns name of currently activated theme for the application.
    QString currentIconTheme() const;

    // Sets icon theme with given name as the active one and loads it.
    void setCurrentIconTheme(const QString &theme_name);
};

inline QString IconFactory::currentIconTheme() const
{
    return QIcon::themeName();
}

#endif // ICONFACTORY_H
