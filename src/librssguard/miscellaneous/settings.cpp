// For license of this file, see <project-root-folder>/LICENSE.md.

#include "miscellaneous/settings.h"

#include "miscellaneous/application.h"
#include "miscellaneous/iofactory.h"
#include <QDebug>
#include <QDir>
#include <QLocale>
#include <QPointer>

#if defined (USE_WEBENGINE)

// WebEngine.
DKEY WebEngineAttributes::ID = "web_engine_attributes";
#endif

// AdBlock.
DKEY AdBlock::ID = "adblock";
DKEY AdBlock::DisabledRules = "disabled_rules";

DVALUE(QStringList) AdBlock::DisabledRulesDef = QStringList();

DKEY AdBlock::AdBlockEnabled = "enabled";

DVALUE(bool) AdBlock::AdBlockEnabledDef = false;

DKEY AdBlock::LastUpdatedOn = "last_updated_on";

DVALUE(QDateTime) AdBlock::LastUpdatedOnDef = QDateTime();

// Feeds.
DKEY Feeds::ID = "feeds";
DKEY Feeds::UpdateTimeout = "feed_update_timeout";

DVALUE(int) Feeds::UpdateTimeoutDef = DOWNLOAD_TIMEOUT;

DKEY Feeds::EnableAutoUpdateNotification = "enable_auto_update_notification";

DVALUE(bool) Feeds::EnableAutoUpdateNotificationDef = true;

DKEY Feeds::CountFormat = "count_format";

DVALUE(char *) Feeds::CountFormatDef = "(%unread)";

DKEY Feeds::AutoUpdateInterval = "auto_update_interval";

DVALUE(int) Feeds::AutoUpdateIntervalDef = DEFAULT_AUTO_UPDATE_INTERVAL;

DKEY Feeds::AutoUpdateEnabled = "auto_update_enabled";

DVALUE(bool) Feeds::AutoUpdateEnabledDef = false;

DKEY Feeds::AutoUpdateOnlyUnfocused = "auto_update_only_unfocused";

DVALUE(bool) Feeds::AutoUpdateOnlyUnfocusedDef = false;

DKEY Feeds::FeedsUpdateOnStartup = "feeds_update_on_startup";

DVALUE(bool) Feeds::FeedsUpdateOnStartupDef = false;

DKEY Feeds::FeedsUpdateStartupDelay = "feeds_update_on_startup_delay";

DVALUE(double) Feeds::FeedsUpdateStartupDelayDef = STARTUP_UPDATE_DELAY;

DKEY Feeds::ShowOnlyUnreadFeeds = "show_only_unread_feeds";

DVALUE(bool) Feeds::ShowOnlyUnreadFeedsDef = false;

DKEY Feeds::ShowTreeBranches = "show_tree_branches";

DVALUE(bool) Feeds::ShowTreeBranchesDef = true;

DKEY Feeds::ListFont = "list_font";

// Messages.
DKEY Messages::ID = "messages";
DKEY Messages::MessageHeadImageHeight = "message_head_image_height";

DVALUE(int) Messages::MessageHeadImageHeightDef = 36;

DKEY Messages::EnableMessagePreview = "enable_message_preview";

DVALUE(bool) Messages::EnableMessagePreviewDef = true;

#if !defined (USE_WEBENGINE)
DKEY Messages::DisplayImagePlaceholders = "display_image_placeholders";

DVALUE(bool) Messages::DisplayImagePlaceholdersDef = false;
#endif

DKEY Messages::Zoom = "zoom";

DVALUE(qreal) Messages::ZoomDef = double(1.0f);

DKEY Messages::UseCustomDate = "use_custom_date";

DVALUE(bool) Messages::UseCustomDateDef = false;

DKEY Messages::CustomDateFormat = "custom_date_format";

DVALUE(char *) Messages::CustomDateFormatDef = "";

DKEY Messages::ClearReadOnExit = "clear_read_on_exit";

DVALUE(bool) Messages::ClearReadOnExitDef = false;

DKEY Messages::KeepCursorInCenter = "keep_cursor_center";

DVALUE(bool) Messages::KeepCursorInCenterDef = false;

DKEY Messages::ShowOnlyUnreadMessages = "show_only_unread_messages";
DVALUE(bool) Messages::ShowOnlyUnreadMessagesDef = false;

DKEY Messages::PreviewerFontStandard = "previewer_font_standard";

NON_CONST_DVALUE(QString) Messages::PreviewerFontStandardDef = QFont(QFont().family(),
        12).toString();

DKEY Messages::ListFont = "list_font";

// GUI.
DKEY GUI::ID = "gui";
DKEY GUI::MessageViewState = "msg_view_state";

DVALUE(QString) GUI::MessageViewStateDef = QString();

DKEY GUI::SplitterFeeds = "splitter_feeds";

DVALUE(char *) GUI::SplitterFeedsDef = "";

DKEY GUI::SplitterMessages = "splitter_messages";

DVALUE(char *) GUI::SplitterMessagesDef = "";

DKEY GUI::ToolbarStyle = "toolbar_style";

DVALUE(Qt::ToolButtonStyle) GUI::ToolbarStyleDef = Qt::ToolButtonIconOnly;

DKEY GUI::HeightRowMessages = "height_row_messages";

DVALUE(int) GUI::HeightRowMessagesDef = -1;

DKEY GUI::HeightRowFeeds = "height_row_feeds";

DVALUE(int) GUI::HeightRowFeedsDef = -1;

DKEY GUI::FeedsToolbarActions = "feeds_toolbar";

DVALUE(char *) GUI::FeedsToolbarActionsDef =
    "m_actionUpdateAllItems,m_actionStopRunningItemsUpdate,m_actionMarkAllItemsRead";

DKEY GUI::StatusbarActions = "status_bar";

DVALUE(char *) GUI::StatusbarActionsDef =
    "m_lblProgressFeedsAction,m_barProgressFeedsAction,m_actionUpdateAllItems,m_actionUpdateSelectedItems,m_actionStopRunningItemsUpdate,m_actionFullscreen,m_actionQuit";

DKEY GUI::MainWindowInitialSize = "window_size";
DKEY GUI::MainWindowInitialPosition = "window_position";
DKEY GUI::IsMainWindowMaximizedBeforeFullscreen = "is_window_maximized_before_fullscreen";

DVALUE(bool) GUI::IsMainWindowMaximizedBeforeFullscreenDef = false;

DKEY GUI::MainWindowStartsFullscreen = "start_in_fullscreen";

DVALUE(bool) GUI::MainWindowStartsFullscreenDef = false;

DKEY GUI::MainWindowStartsHidden = "start_hidden";

DVALUE(bool) GUI::MainWindowStartsHiddenDef = false;

DKEY GUI::MainWindowStartsMaximized = "window_is_maximized";

DVALUE(bool) GUI::MainWindowStartsMaximizedDef = false;

DKEY GUI::MainMenuVisible = "main_menu_visible";

DVALUE(bool) GUI::MainMenuVisibleDef = true;

DKEY GUI::ToolbarsVisible = "enable_toolbars";

DVALUE(bool) GUI::ToolbarsVisibleDef = true;

DKEY GUI::ListHeadersVisible = "enable_list_headers";

DVALUE(bool) GUI::ListHeadersVisibleDef = true;

DKEY GUI::StatusBarVisible = "enable_status_bar";

DVALUE(bool) GUI::StatusBarVisibleDef = true;

DKEY GUI::HideMainWindowWhenMinimized = "hide_when_minimized";

DVALUE(bool) GUI::HideMainWindowWhenMinimizedDef = false;

DKEY GUI::MonochromeTrayIcon = "monochrome_tray_icon";

DVALUE(bool) GUI::MonochromeTrayIconDef = false;

DKEY GUI::UseTrayIcon = "use_tray_icon";

DVALUE(bool) GUI::UseTrayIconDef = true;

DKEY GUI::EnableNotifications = "enable_notifications";

DVALUE(bool) GUI::EnableNotificationsDef = true;

DKEY GUI::TabCloseMiddleClick = "tab_close_mid_button";

DVALUE(bool) GUI::TabCloseMiddleClickDef = true;

DKEY GUI::TabCloseDoubleClick = "tab_close_double_button";

DVALUE(bool) GUI::TabCloseDoubleClickDef = true;

DKEY GUI::TabNewDoubleClick = "tab_new_double_button";

DVALUE(bool) GUI::TabNewDoubleClickDef = true;

DKEY GUI::HideTabBarIfOnlyOneTab = "hide_tabbar_one_tab";

DVALUE(bool) GUI::HideTabBarIfOnlyOneTabDef = false;

DKEY GUI::MessagesToolbarDefaultButtons = "messages_toolbar";

DVALUE(char *) GUI::MessagesToolbarDefaultButtonsDef =
    "m_actionMarkSelectedMessagesAsRead,m_actionMarkSelectedMessagesAsUnread,m_actionSwitchImportanceOfSelectedMessages,separator,highlighter,spacer,search";

DKEY GUI::DefaultSortColumnFeeds = "default_sort_column_feeds";

DVALUE(int) GUI::DefaultSortColumnFeedsDef = FDS_MODEL_TITLE_INDEX;

DKEY GUI::DefaultSortOrderFeeds = "default_sort_order_feeds";

DVALUE(Qt::SortOrder) GUI::DefaultSortOrderFeedsDef = Qt::AscendingOrder;

DKEY GUI::IconTheme = "icon_theme_name";

DVALUE(char *) GUI::IconThemeDef = APP_THEME_DEFAULT;

DKEY GUI::Skin = "skin";

DVALUE(char *) GUI::SkinDef = APP_SKIN_DEFAULT;

DKEY GUI::Style = "style";

DVALUE(char *) GUI::StyleDef = APP_STYLE_DEFAULT;

// General.
DKEY General::ID = "main";
DKEY General::UpdateOnStartup = "update_on_start";

DVALUE(bool) General::UpdateOnStartupDef = true;

DKEY General::RemoveTrolltechJunk = "remove_trolltech_junk";

DVALUE(bool) General::RemoveTrolltechJunkDef = false;

DKEY General::FirstRun = "first_run";

DVALUE(bool) General::FirstRunDef = true;

DKEY General::Language = "language";

DVALUE(QString) General::LanguageDef = QLocale::system().name();

// Downloads.
DKEY Downloads::ID = "download_manager";
DKEY Downloads::AlwaysPromptForFilename = "prompt_for_filename";

DVALUE(bool) Downloads::AlwaysPromptForFilenameDef = false;

DKEY Downloads::TargetDirectory = "target_directory";

DVALUE(QString) Downloads::TargetDirectoryDef = IOFactory::getSystemFolder(
            QStandardPaths::DownloadLocation);

DKEY Downloads::RemovePolicy = "remove_policy";

DVALUE(int) Downloads::RemovePolicyDef = int(DownloadManager::RemovePolicy::Never);

DKEY Downloads::TargetExplicitDirectory = "target_explicit_directory";

DVALUE(QString) Downloads::TargetExplicitDirectoryDef = IOFactory::getSystemFolder(
            QStandardPaths::DownloadLocation);

DKEY Downloads::ShowDownloadsWhenNewDownloadStarts = "show_downloads_on_new_download_start";

DVALUE(bool) Downloads::ShowDownloadsWhenNewDownloadStartsDef = true;

DKEY Downloads::ItemUrl = "download_%1_url";
DKEY Downloads::ItemLocation = "download_%1_location";
DKEY Downloads::ItemDone = "download_%1_done";

// Proxy.
DKEY Proxy::ID = "proxy";
DKEY Proxy::Type = "proxy_type";

DVALUE(QNetworkProxy::ProxyType) Proxy::TypeDef = QNetworkProxy::NoProxy;

DKEY Proxy::Host = "host";

DVALUE(QString) Proxy::HostDef = QString();

DKEY Proxy::Username = "username";

DVALUE(QString) Proxy::UsernameDef = QString();

DKEY Proxy::Password = "password";

DVALUE(QString) Proxy::PasswordDef = QString();

DKEY Proxy::Port = "port";

DVALUE(int) Proxy::PortDef = 80;

// Database.
DKEY Database::ID = "database";
DKEY Database::UseTransactions = "use_transactions";

DVALUE(bool) Database::UseTransactionsDef = false;

DKEY Database::UseInMemory = "use_in_memory_db";

DVALUE(bool) Database::UseInMemoryDef = false;

DKEY Database::MySQLHostname = "mysql_hostname";

DVALUE(QString) Database::MySQLHostnameDef = QString();

DKEY Database::MySQLUsername = "mysql_username";

DVALUE(QString) Database::MySQLUsernameDef = QString();

DKEY Database::MySQLPassword = "mysql_password";

DVALUE(QString) Database::MySQLPasswordDef = QString();

DKEY Database::MySQLDatabase = "mysql_database";

DVALUE(char *) Database::MySQLDatabaseDef = APP_LOW_NAME;

DKEY Database::MySQLPort = "mysql_port";

DVALUE(int) Database::MySQLPortDef = APP_DB_MYSQL_PORT;

DKEY Database::ActiveDriver = "database_driver";

DVALUE(char *) Database::ActiveDriverDef = APP_DB_SQLITE_DRIVER;

// Keyboard.
DKEY Keyboard::ID = "keyboard";

// Web browser.
DKEY Browser::ID = "browser";
DKEY Browser::SendDNT = "send_dnt";

VALUE(bool) Browser::SendDNTDef = false;

DKEY Browser::OpenLinksInExternalBrowserRightAway = "open_link_externally_wo_confirmation";

DVALUE(bool) Browser::OpenLinksInExternalBrowserRightAwayDef = false;

DKEY Browser::CustomExternalBrowserEnabled = "custom_external_browser";

DVALUE(bool) Browser::CustomExternalBrowserEnabledDef = false;

DKEY Browser::CustomExternalBrowserExecutable = "external_browser_executable";

DVALUE(QString) Browser::CustomExternalBrowserExecutableDef = QString();

DKEY Browser::CustomExternalBrowserArguments = "external_browser_arguments";

DVALUE(char *) Browser::CustomExternalBrowserArgumentsDef = "%1";

DKEY Browser::CustomExternalEmailEnabled = "custom_external_email";

DVALUE(bool) Browser::CustomExternalEmailEnabledDef = false;

DKEY Browser::CustomExternalEmailExecutable = "external_email_executable";

DVALUE(QString) Browser::CustomExternalEmailExecutableDef = QString();

DKEY Browser::CustomExternalEmailArguments = "external_email_arguments";

DVALUE(char *) Browser::CustomExternalEmailArgumentsDef = "";

DKEY Browser::ExternalTools = "external_tools";

DVALUE(QStringList) Browser::ExternalToolsDef = QStringList();

// Categories.
DKEY CategoriesExpandStates::ID = "categories_expand_states";

Settings::Settings(const QString &file_name, Format format,
                   const SettingsProperties::SettingsType &type, QObject *parent)
    : QSettings(file_name, format, parent), m_initializationStatus(type) {}

Settings::~Settings() = default;

QString Settings::pathName() const
{
    return QFileInfo(fileName()).absolutePath();
}

QSettings::Status Settings::checkSettings()
{
    qDebugNN << LOGSEC_CORE << "Syncing settings.";
    sync();
    return status();
}

bool Settings::initiateRestoration(const QString &settings_backup_file_path)
{
    return IOFactory::copyFile(settings_backup_file_path,
                               QFileInfo(fileName()).absolutePath() + QDir::separator() +
                               BACKUP_NAME_SETTINGS + BACKUP_SUFFIX_SETTINGS);
}

void Settings::finishRestoration(const QString &desired_settings_file_path)
{
    const QString backup_settings_file = QFileInfo(desired_settings_file_path).absolutePath() +
                                         QDir::separator() +
                                         BACKUP_NAME_SETTINGS + BACKUP_SUFFIX_SETTINGS;

    if (QFile::exists(backup_settings_file)) {
        qWarningNN << LOGSEC_CORE
                   << "Backup settings file"
                   << QUOTE_W_SPACE(QDir::toNativeSeparators(backup_settings_file))
                   << "was detected. Restoring it.";

        if (IOFactory::copyFile(backup_settings_file, desired_settings_file_path)) {
            QFile::remove(backup_settings_file);
            qDebugNN << LOGSEC_CORE << "Settings file was restored successully.";
        } else {
            qCriticalNN << LOGSEC_CORE << "Settings file was NOT restored due to error when copying the file.";
        }
    }
}

Settings *Settings::setupSettings(QObject *parent)
{
    Settings *new_settings;

    // If settings file exists (and is writable) in executable file working directory
    // (in subdirectory APP_CFG_PATH), then use it (portable settings).
    // Otherwise use settings file stored in home path.
    const SettingsProperties properties = determineProperties();

    finishRestoration(properties.m_absoluteSettingsFileName);

    // Portable settings are available, use them.
    new_settings = new Settings(properties.m_absoluteSettingsFileName, QSettings::IniFormat,
                                properties.m_type, parent);

    // Check if portable settings are available.
    if (properties.m_type == SettingsProperties::SettingsType::Portable) {
        qDebugNN << LOGSEC_CORE
                 << "Initializing settings in"
                 << QUOTE_W_SPACE(QDir::toNativeSeparators(properties.m_absoluteSettingsFileName))
                 << "(portable way).";
    } else {
        qDebugNN << LOGSEC_CORE
                 << "Initializing settings in"
                 << QUOTE_W_SPACE(QDir::toNativeSeparators(properties.m_absoluteSettingsFileName))
                 << "(non-portable way).";
    }

    return new_settings;
}

SettingsProperties Settings::determineProperties()
{
    SettingsProperties properties;

    properties.m_settingsSuffix = QDir::separator() + QSL(APP_CFG_PATH) + QDir::separator() + QSL(
                                      APP_CFG_FILE);
    const QString app_path = qApp->userDataAppFolder();
    const QString home_path = qApp->userDataHomeFolder();

    // We will use PORTABLE settings only and only if it is available and NON-PORTABLE
    // settings was not initialized before.
#if defined (Q_OS_LINUX) || defined (Q_OS_ANDROID) || defined (Q_OS_MACOSOS)
    // DO NOT use portable settings for Linux, it is really not used on that platform.
    const bool will_we_use_portable_settings = false;
#else
    const QString exe_path = qApp->applicationDirPath();
    const QString home_path_file = home_path + properties.m_settingsSuffix;
    const bool portable_settings_available = IOFactory::isFolderWritable(exe_path);
    const bool non_portable_settings_exist = QFile::exists(home_path_file);
    const bool will_we_use_portable_settings = portable_settings_available
            && !non_portable_settings_exist;
#endif

    if (will_we_use_portable_settings) {
        properties.m_type = SettingsProperties::SettingsType::Portable;
        properties.m_baseDirectory = app_path;
    } else {
        properties.m_type = SettingsProperties::SettingsType::NonPortable;
        properties.m_baseDirectory = home_path;
    }

    properties.m_absoluteSettingsFileName = properties.m_baseDirectory + properties.m_settingsSuffix;
    return properties;
}
