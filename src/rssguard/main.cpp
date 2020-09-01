// For license of this file, see <project-root-folder>/LICENSE.md.

#include "core/feedsmodel.h"
#include "definitions/definitions.h"
#include "gui/dialogs/formmain.h"
#include "gui/feedmessageviewer.h"
#include "gui/feedsview.h"
#include "miscellaneous/application.h"

#if defined (Q_OS_MACOS)
extern void disableWindowTabbing();

#endif

int main(int argc, char *argv[])
{
    for (int i = 0; i < argc; i++) {
        const QString str = QString::fromLocal8Bit(argv[i]);

        if (str == "-h") {
            qDebug("Usage: rssguard [OPTIONS]\n\n"
                   "Option\t\tMeaning\n"
                   "-h\t\tDisplays this help.");
            return EXIT_SUCCESS;
        }
    }

    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

#if defined (Q_OS_LINUX)
    QApplication::setDesktopFileName(APP_DESKTOP_ENTRY_FILE);
#endif

    // Ensure that ini format is used as application settings storage on Mac OS.
    QSettings::setDefaultFormat(QSettings::IniFormat);

#if defined (Q_OS_MACOS)
    QApplication::setAttribute(Qt::AA_DontShowIconsInMenus);
    disableWindowTabbing();
#endif

    // Instantiate base application object.
    Application application(APP_LOW_NAME, argc, argv);

    qDebug("Starting %s.", qPrintable(QSL(APP_LONG_NAME)));
    qDebug("Instantiated Application class.");

    // Check if another instance is running.
    if (application.isAlreadyRunning()) {
        qWarning("Another instance of the application is already running. Notifying it.");
        return EXIT_FAILURE;
    }

    // Load localization and setup locale before any widget is constructed.
    qApp->localization()->loadActiveLanguage();
    qApp->setFeedReader(new FeedReader(&application));

    // Register needed metatypes.
    qRegisterMetaType<QList<Message>>("QList<Message>");
    qRegisterMetaType<QList<RootItem *>>("QList<RootItem*>");

    // Add an extra path for non-system icon themes and set current icon theme
    // and skin.
    qApp->icons()->setupSearchPaths();
    qApp->icons()->loadCurrentIconTheme();
    qApp->skins()->loadCurrentSkin();

    // These settings needs to be set before any QSettings object.
    Application::setApplicationName(APP_NAME);
    Application::setApplicationVersion(APP_VERSION);
    Application::setOrganizationDomain(APP_URL);
    Application::setWindowIcon(QIcon(APP_ICON_PATH));

    qApp->reactOnForeignNotifications();

    FormMain main_window;

    qApp->loadDynamicShortcuts();
    qApp->hideOrShowMainForm();
    qApp->feedReader()->loadSavedMessageFilters();
    qApp->feedReader()->feedsModel()->loadActivatedServiceAccounts();
    qApp->showTrayIcon();
    qApp->offerChanges();
    qApp->showPolls();
    qApp->mainForm()->tabWidget()->feedMessageViewer()->feedsView()->loadAllExpandStates();

    return Application::exec();
}
