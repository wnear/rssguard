// For license of this file, see <project-root-folder>/LICENSE.md.

#ifndef SETTINGS_H
#define SETTINGS_H

#include <QSettings>

#include "definitions/definitions.h"

#include "miscellaneous/settingsproperties.h"
#include "miscellaneous/textfactory.h"

#include <QByteArray>
#include <QColor>
#include <QDateTime>
#include <QNetworkProxy>
#include <QStringList>

#define KEY extern const char*
#define DKEY const char*
#define VALUE(x) extern const x
#define NON_CONST_VALUE(x) extern x
#define DVALUE(x) const x
#define NON_CONST_DVALUE(x) x
#define SETTING(x) x, x ## Def
#define DEFAULT_VALUE(x) x ## Def
#define GROUP(x) x::ID

#if defined (USE_WEBENGINE)
namespace WebEngineAttributes
{
KEY ID;
}
#endif

namespace AdBlock
{
KEY ID;
KEY AdBlockEnabled;

VALUE(bool) AdBlockEnabledDef;

KEY DisabledRules;

VALUE(QStringList) DisabledRulesDef;

KEY LastUpdatedOn;

VALUE(QDateTime) LastUpdatedOnDef;
}

// Feeds.
namespace Feeds
{
KEY ID;
KEY UpdateTimeout;

VALUE(int) UpdateTimeoutDef;

KEY EnableAutoUpdateNotification;

VALUE(bool) EnableAutoUpdateNotificationDef;

KEY CountFormat;

VALUE(char *) CountFormatDef;

KEY AutoUpdateInterval;

VALUE(int) AutoUpdateIntervalDef;

KEY AutoUpdateEnabled;

VALUE(bool) AutoUpdateEnabledDef;

KEY AutoUpdateOnlyUnfocused;

VALUE(bool) AutoUpdateOnlyUnfocusedDef;

KEY FeedsUpdateOnStartup;

VALUE(bool) FeedsUpdateOnStartupDef;

KEY FeedsUpdateStartupDelay;

VALUE(double) FeedsUpdateStartupDelayDef;

KEY ShowOnlyUnreadFeeds;

VALUE(bool) ShowOnlyUnreadFeedsDef;

KEY ShowTreeBranches;

VALUE(bool) ShowTreeBranchesDef;

KEY ListFont;
}

// Messages.
namespace Messages
{
KEY ID;
KEY MessageHeadImageHeight;

VALUE(int) MessageHeadImageHeightDef;

KEY EnableMessagePreview;

VALUE(bool) EnableMessagePreviewDef;

#if !defined (USE_WEBENGINE)
KEY DisplayImagePlaceholders;

VALUE(bool) DisplayImagePlaceholdersDef;
#endif

KEY Zoom;

VALUE(qreal) ZoomDef;

KEY UseCustomDate;

VALUE(bool) UseCustomDateDef;

KEY CustomDateFormat;
VALUE(char *) CustomDateFormatDef;

KEY ClearReadOnExit;
VALUE(bool) ClearReadOnExitDef;

KEY KeepCursorInCenter;
VALUE(bool) KeepCursorInCenterDef;

KEY ShowOnlyUnreadMessages;
VALUE(bool) ShowOnlyUnreadMessagesDef;

KEY PreviewerFontStandard;
NON_CONST_VALUE(QString) PreviewerFontStandardDef;

KEY ListFont;
}

// GUI.
namespace GUI
{
KEY ID;
KEY MessageViewState;

VALUE(QString) MessageViewStateDef;

KEY SplitterFeeds;

VALUE(char *) SplitterFeedsDef;

KEY SplitterMessages;

VALUE(char *) SplitterMessagesDef;

KEY ToolbarStyle;

VALUE(Qt::ToolButtonStyle) ToolbarStyleDef;

KEY FeedsToolbarActions;

VALUE(char *) FeedsToolbarActionsDef;

KEY StatusbarActions;

VALUE(char *) StatusbarActionsDef;

KEY MainWindowInitialSize;
KEY MainWindowInitialPosition;
KEY IsMainWindowMaximizedBeforeFullscreen;

VALUE(bool) IsMainWindowMaximizedBeforeFullscreenDef;

KEY MainWindowStartsFullscreen;

VALUE(bool) MainWindowStartsFullscreenDef;

KEY MainWindowStartsHidden;

VALUE(bool) MainWindowStartsHiddenDef;

KEY MainWindowStartsMaximized;

VALUE(bool) MainWindowStartsMaximizedDef;

KEY MainMenuVisible;

VALUE(bool) MainMenuVisibleDef;

KEY ToolbarsVisible;

VALUE(bool) ToolbarsVisibleDef;

KEY ListHeadersVisible;

VALUE(bool) ListHeadersVisibleDef;

KEY StatusBarVisible;

VALUE(bool) StatusBarVisibleDef;

KEY HideMainWindowWhenMinimized;

VALUE(bool) HideMainWindowWhenMinimizedDef;

KEY UseTrayIcon;

VALUE(bool) UseTrayIconDef;

KEY MonochromeTrayIcon;

VALUE(bool) MonochromeTrayIconDef;

KEY EnableNotifications;

VALUE(bool) EnableNotificationsDef;

KEY TabCloseMiddleClick;

VALUE(bool) TabCloseMiddleClickDef;

KEY TabCloseDoubleClick;

VALUE(bool) TabCloseDoubleClickDef;

KEY TabNewDoubleClick;

VALUE(bool) TabNewDoubleClickDef;

KEY HideTabBarIfOnlyOneTab;

VALUE(bool) HideTabBarIfOnlyOneTabDef;

KEY MessagesToolbarDefaultButtons;

VALUE(char *) MessagesToolbarDefaultButtonsDef;

KEY DefaultSortColumnFeeds;

VALUE(int) DefaultSortColumnFeedsDef;

KEY HeightRowMessages;

VALUE(int) HeightRowMessagesDef;

KEY HeightRowFeeds;

VALUE(int) HeightRowFeedsDef;

KEY DefaultSortOrderFeeds;

VALUE(Qt::SortOrder) DefaultSortOrderFeedsDef;

KEY IconTheme;

VALUE(char *) IconThemeDef;

KEY Skin;

VALUE(char *) SkinDef;

KEY Style;

VALUE(char *) StyleDef;
}

// General.
namespace General
{
KEY ID;
KEY UpdateOnStartup;

VALUE(bool) UpdateOnStartupDef;

KEY RemoveTrolltechJunk;

VALUE(bool) RemoveTrolltechJunkDef;

KEY FirstRun;

VALUE(bool) FirstRunDef;

KEY Language;

VALUE(QString) LanguageDef;
}

// Downloads.
namespace Downloads
{
KEY ID;
KEY AlwaysPromptForFilename;

VALUE(bool) AlwaysPromptForFilenameDef;

KEY TargetDirectory;

VALUE(QString) TargetDirectoryDef;

KEY RemovePolicy;

VALUE(int) RemovePolicyDef;

KEY TargetExplicitDirectory;

VALUE(QString) TargetExplicitDirectoryDef;

KEY ShowDownloadsWhenNewDownloadStarts;

VALUE(bool) ShowDownloadsWhenNewDownloadStartsDef;

KEY ItemUrl;
KEY ItemLocation;
KEY ItemDone;
}

// Proxy.
namespace Proxy
{
KEY ID;
KEY Type;

VALUE(QNetworkProxy::ProxyType) TypeDef;

KEY Host;

VALUE(QString) HostDef;

KEY Username;

VALUE(QString) UsernameDef;

KEY Password;

VALUE(QString) PasswordDef;

KEY Port;

VALUE(int) PortDef;
}

// Database.
namespace Database
{
KEY ID;
KEY UseTransactions;

VALUE(bool) UseTransactionsDef;

KEY UseInMemory;

VALUE(bool) UseInMemoryDef;

KEY MySQLHostname;

VALUE(QString) MySQLHostnameDef;

KEY MySQLUsername;

VALUE(QString) MySQLUsernameDef;

KEY MySQLPassword;

VALUE(QString) MySQLPasswordDef;

KEY MySQLPort;

VALUE(int) MySQLPortDef;

KEY MySQLDatabase;

VALUE(char *) MySQLDatabaseDef;

KEY ActiveDriver;

VALUE(char *) ActiveDriverDef;
}

// Keyboard.
namespace Keyboard
{
KEY ID;
}

// Web browser.
namespace Browser
{
KEY ID;
KEY SendDNT;

VALUE(bool) SendDNTDef;

KEY OpenLinksInExternalBrowserRightAway;

VALUE(bool) OpenLinksInExternalBrowserRightAwayDef;

KEY CustomExternalBrowserEnabled;

VALUE(bool) CustomExternalBrowserEnabledDef;

KEY CustomExternalBrowserExecutable;

VALUE(QString) CustomExternalBrowserExecutableDef;

KEY CustomExternalBrowserArguments;

VALUE(char *) CustomExternalBrowserArgumentsDef;

KEY CustomExternalEmailEnabled;

VALUE(bool) CustomExternalEmailEnabledDef;

KEY CustomExternalEmailExecutable;

VALUE(QString) CustomExternalEmailExecutableDef;

KEY ExternalTools;

VALUE(QStringList) ExternalToolsDef;

KEY CustomExternalEmailArguments;

VALUE(char *) CustomExternalEmailArgumentsDef;
}

// Categories.
namespace CategoriesExpandStates
{
KEY ID;
}

class Settings : public QSettings
{
    Q_OBJECT

public:

    // Destructor.
    virtual ~Settings();

    // Type of used settings.
    SettingsProperties::SettingsType type() const;

    // Getters/setters for settings values.
    QVariant password(const QString &section, const QString &key,
                      const QVariant &default_value = QVariant()) const;
    void setPassword(const QString &section, const QString &key, const QVariant &value);

    QVariant value(const QString &section, const QString &key,
                   const QVariant &default_value = QVariant()) const;
    void setValue(const QString &section, const QString &key, const QVariant &value);
    void setValue(const QString &key, const QVariant &value);

    bool contains(const QString &section, const QString &key) const;
    void remove(const QString &section, const QString &key);

    // Returns the path which contains the settings.
    QString pathName() const;

    // Synchronizes settings.
    QSettings::Status checkSettings();

    bool initiateRestoration(const QString &settings_backup_file_path);
    static void finishRestoration(const QString &desired_settings_file_path);

    // Creates settings file in correct location.
    static Settings *setupSettings(QObject *parent);

    // Returns properties of the actual application-wide settings.
    static SettingsProperties determineProperties();

private:

    // Constructor.
    explicit Settings(const QString &file_name, Format format,
                      const SettingsProperties::SettingsType &type, QObject *parent = nullptr);

    SettingsProperties::SettingsType m_initializationStatus;
};

inline SettingsProperties::SettingsType Settings::type() const
{
    return m_initializationStatus;
}

// Getters/setters for settings values.
inline QVariant Settings::password(const QString &section, const QString &key,
                                   const QVariant &default_value) const
{
    return TextFactory::decrypt(value(section, key, default_value).toString());
}

inline void Settings::setPassword(const QString &section, const QString &key,
                                  const QVariant &value)
{
    setValue(section, key, TextFactory::encrypt(value.toString()));
}

inline QVariant Settings::value(const QString &section, const QString &key,
                                const QVariant &default_value) const
{
    return QSettings::value(QString(QSL("%1/%2")).arg(section, key), default_value);
}

inline void Settings::setValue(const QString &section, const QString &key, const QVariant &value)
{
    QSettings::setValue(QString(QSL("%1/%2")).arg(section, key), value);
}

inline void Settings::setValue(const QString &key, const QVariant &value)
{
    QSettings::setValue(key, value);
}

inline bool Settings::contains(const QString &section, const QString &key) const
{
    return QSettings::contains(QString(QSL("%1/%2")).arg(section, key));
}

inline void Settings::remove(const QString &section, const QString &key)
{
    QSettings::remove(QString(QSL("%1/%2")).arg(section, key));
}

#endif // SETTINGS_H
