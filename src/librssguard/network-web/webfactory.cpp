// For license of this file, see <project-root-folder>/LICENSE.md.

#include "network-web/webfactory.h"

#include "miscellaneous/application.h"
#include "miscellaneous/iconfactory.h"

#include <QDesktopServices>
#include <QProcess>
#include <QUrl>

#if defined (USE_WEBENGINE)
#include <QWebEngineProfile>
#endif

WebFactory::WebFactory(QObject *parent)
    : QObject(parent)
{
#if defined (USE_WEBENGINE)
    m_engineSettings = nullptr;
#endif
}

WebFactory::~WebFactory()
{
#if defined (USE_WEBENGINE)
    if (m_engineSettings != nullptr && m_engineSettings->menu() != nullptr) {
        m_engineSettings->menu()->deleteLater();
    }
#endif
}

bool WebFactory::sendMessageViaEmail(const Message &message)
{
    if (qApp->settings()->value(GROUP(Browser),
                                SETTING(Browser::CustomExternalEmailEnabled)).toBool()) {
        const QString browser = qApp->settings()->value(GROUP(Browser),
                                SETTING(Browser::CustomExternalEmailExecutable)).toString();
        const QString arguments = qApp->settings()->value(GROUP(Browser),
                                  SETTING(Browser::CustomExternalEmailArguments)).toString();

        return IOFactory::startProcessDetached(browser, {}, arguments.arg(message.m_title,
                                               stripTags(message.m_contents)));
    } else {
        // Send it via mailto protocol.
        // NOTE: http://en.wikipedia.org/wiki/Mailto
        return QDesktopServices::openUrl(QString("mailto:?subject=%1&body=%2").arg(QString(
                                             QUrl::toPercentEncoding(message.m_title)),
                                         QString(QUrl::toPercentEncoding(stripTags(
                                                 message.m_contents)))));
    }
}

bool WebFactory::openUrlInExternalBrowser(const QString &url) const
{
    if (qApp->settings()->value(GROUP(Browser),
                                SETTING(Browser::CustomExternalBrowserEnabled)).toBool()) {
        const QString browser = qApp->settings()->value(GROUP(Browser),
                                SETTING(Browser::CustomExternalBrowserExecutable)).toString();
        const QString arguments = qApp->settings()->value(GROUP(Browser),
                                  SETTING(Browser::CustomExternalBrowserArguments)).toString();
        auto nice_args = arguments.arg(url);

        qDebugNN << LOGSEC_NETWORK << "Arguments for external browser:" << QUOTE_W_SPACE_DOT(nice_args);

        const bool result = IOFactory::startProcessDetached(browser, {}, nice_args);

        if (!result) {
            qDebugNN << LOGSEC_NETWORK << "External web browser call failed.";
        }

        return result;
    } else {
        return QDesktopServices::openUrl(url);
    }
}

QString WebFactory::stripTags(QString text)
{
    return text.remove(QRegularExpression(QSL("<[^>]*>")));
}

QString WebFactory::escapeHtml(const QString &html)
{
    if (m_escapes.isEmpty()) {
        generateEscapes();
    }

    QString output = html;
    QMapIterator<QString, QString> i(m_escapes);

    while (i.hasNext()) {
        i.next();
        output = output.replace(i.key(), i.value());
    }

    return output;
}

QString WebFactory::deEscapeHtml(const QString &text)
{
    if (m_deEscapes.isEmpty()) {
        generateDeescapes();
    }

    QString output = text;
    QMapIterator<QString, QString> i(m_deEscapes);

    while (i.hasNext()) {
        i.next();
        output = output.replace(i.key(), i.value());
    }

    return output;
}

void WebFactory::updateProxy()
{
    const QNetworkProxy::ProxyType selected_proxy_type = static_cast<QNetworkProxy::ProxyType>
            (qApp->settings()->value(GROUP(Proxy),
                                     SETTING(Proxy::Type)).
             toInt());

    if (selected_proxy_type == QNetworkProxy::NoProxy) {
        QNetworkProxyFactory::setUseSystemConfiguration(false);
        QNetworkProxy::setApplicationProxy(QNetworkProxy::NoProxy);
    } else if (selected_proxy_type == QNetworkProxy::DefaultProxy) {
        QNetworkProxyFactory::setUseSystemConfiguration(true);
    } else {
        const Settings *settings = qApp->settings();
        QNetworkProxy new_proxy;

        // Custom proxy is selected, set it up.
        new_proxy.setType(selected_proxy_type);
        new_proxy.setHostName(settings->value(GROUP(Proxy), SETTING(Proxy::Host)).toString());
        new_proxy.setPort(quint16(settings->value(GROUP(Proxy), SETTING(Proxy::Port)).toInt()));
        new_proxy.setUser(settings->value(GROUP(Proxy), SETTING(Proxy::Username)).toString());
        new_proxy.setPassword(settings->password(GROUP(Proxy), SETTING(Proxy::Password)).toString());

        QNetworkProxy::setApplicationProxy(new_proxy);
    }
}

#if defined (USE_WEBENGINE)
QAction *WebFactory::engineSettingsAction()
{
    if (m_engineSettings == nullptr) {
        m_engineSettings = new QAction(qApp->icons()->fromTheme(QSL("applications-internet")),
                                       tr("Web engine settings"), this);
        m_engineSettings->setMenu(new QMenu());
        createMenu(m_engineSettings->menu());
        connect(m_engineSettings->menu(), SIGNAL(aboutToShow()), this, SLOT(createMenu()));
    }

    return m_engineSettings;
}

void WebFactory::createMenu(QMenu *menu)
{
    if (menu == nullptr) {
        menu = qobject_cast<QMenu *>(sender());

        if (menu == nullptr) {
            return;
        }
    }

    menu->clear();
    QList<QAction *> actions;

    actions << createEngineSettingsAction(tr("Auto-load images"), QWebEngineSettings::AutoLoadImages);
    actions << createEngineSettingsAction(tr("JS enabled"), QWebEngineSettings::JavascriptEnabled);
    actions << createEngineSettingsAction(tr("JS can open popup windows"),
                                          QWebEngineSettings::JavascriptCanOpenWindows);
    actions << createEngineSettingsAction(tr("JS can access clipboard"),
                                          QWebEngineSettings::JavascriptCanAccessClipboard);
    actions << createEngineSettingsAction(tr("Hyperlinks can get focus"),
                                          QWebEngineSettings::LinksIncludedInFocusChain);
    actions << createEngineSettingsAction(tr("Local storage enabled"),
                                          QWebEngineSettings::LocalStorageEnabled);
    actions << createEngineSettingsAction(tr("Local content can access remote URLs"),
                                          QWebEngineSettings::LocalContentCanAccessRemoteUrls);
    actions << createEngineSettingsAction(tr("XSS auditing enabled"),
                                          QWebEngineSettings::XSSAuditingEnabled);
    actions << createEngineSettingsAction(tr("Spatial navigation enabled"),
                                          QWebEngineSettings::SpatialNavigationEnabled);
    actions << createEngineSettingsAction(tr("Local content can access local files"),
                                          QWebEngineSettings::LocalContentCanAccessFileUrls);
    actions << createEngineSettingsAction(tr("Hyperlink auditing enabled"),
                                          QWebEngineSettings::HyperlinkAuditingEnabled);
    actions << createEngineSettingsAction(tr("Animate scrolling"),
                                          QWebEngineSettings::ScrollAnimatorEnabled);
    actions << createEngineSettingsAction(tr("Error pages enabled"),
                                          QWebEngineSettings::ErrorPageEnabled);
    actions << createEngineSettingsAction(tr("Plugins enabled"), QWebEngineSettings::PluginsEnabled);
    actions << createEngineSettingsAction(tr("Fullscreen enabled"),
                                          QWebEngineSettings::FullScreenSupportEnabled);

#if !defined(Q_OS_LINUX)
    actions << createEngineSettingsAction(tr("Screen capture enabled"),
                                          QWebEngineSettings::ScreenCaptureEnabled);
    actions << createEngineSettingsAction(tr("WebGL enabled"), QWebEngineSettings::WebGLEnabled);
    actions << createEngineSettingsAction(tr("Accelerate 2D canvas"),
                                          QWebEngineSettings::Accelerated2dCanvasEnabled);
    actions << createEngineSettingsAction(tr("Print element backgrounds"),
                                          QWebEngineSettings::PrintElementBackgrounds);
    actions << createEngineSettingsAction(tr("Allow running insecure content"),
                                          QWebEngineSettings::AllowRunningInsecureContent);
    actions << createEngineSettingsAction(tr("Allow geolocation on insecure origins"),
                                          QWebEngineSettings::AllowGeolocationOnInsecureOrigins);
#endif

    menu->addActions(actions);
}

void WebFactory::webEngineSettingChanged(bool enabled)
{
    const QAction *const act = qobject_cast<QAction *>(sender());

    QWebEngineSettings::WebAttribute attribute = static_cast<QWebEngineSettings::WebAttribute>
            (act->data().toInt());

    qApp->settings()->setValue(WebEngineAttributes::ID, QString::number(static_cast<int>(attribute)),
                               enabled);
    QWebEngineProfile::defaultProfile()->settings()->setAttribute(attribute, act->isChecked());
}

QAction *WebFactory::createEngineSettingsAction(const QString &title,
        QWebEngineSettings::WebAttribute attribute)
{
    auto *act = new QAction(title, m_engineSettings->menu());

    act->setData(attribute);
    act->setCheckable(true);
    act->setChecked(qApp->settings()->value(WebEngineAttributes::ID,
                                            QString::number(static_cast<int>(attribute)), true).toBool());
    QWebEngineProfile::defaultProfile()->settings()->setAttribute(attribute, act->isChecked());
    connect(act, &QAction::toggled, this, &WebFactory::webEngineSettingChanged);
    return act;
}

#endif

void WebFactory::generateEscapes()
{
    m_escapes[QSL("&lt;")] = QL1C('<');
    m_escapes[QSL("&gt;")] = QL1C('>');
    m_escapes[QSL("&amp;")] = QL1C('&');
    m_escapes[QSL("&quot;")] = QL1C('\"');
    m_escapes[QSL("&nbsp;")] = QL1C(' ');
    m_escapes[QSL("&plusmn;")] = QSL("±");
    m_escapes[QSL("&times;")] = QSL("×");
    m_escapes[QSL("&#039;")] = QL1C('\'');
}

void WebFactory::generateDeescapes()
{
    m_deEscapes[QSL("<")] = QSL("&lt;");
    m_deEscapes[QSL(">")] = QSL("&gt;");
    m_deEscapes[QSL("&")] = QSL("&amp;");
    m_deEscapes[QSL("\"")] = QSL("&quot;");
    m_deEscapes[QSL("±")] = QSL("&plusmn;");
    m_deEscapes[QSL("×")] = QSL("&times;");
    m_deEscapes[QSL("\'")] = QSL("&#039;");
}
