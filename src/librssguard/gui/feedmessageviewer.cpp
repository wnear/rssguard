// For license of this file, see <project-root-folder>/LICENSE.md.

#include "gui/feedmessageviewer.h"

#include "core/feeddownloader.h"
#include "core/feedsproxymodel.h"
#include "core/messagesproxymodel.h"
#include "gui/feedstoolbar.h"
#include "gui/feedsview.h"
#include "gui/messagebox.h"
#include "gui/messagestoolbar.h"
#include "gui/messagesview.h"
#include "gui/statusbar.h"
#include "gui/systemtrayicon.h"
#include "miscellaneous/databasecleaner.h"
#include "miscellaneous/databasefactory.h"
#include "miscellaneous/feedreader.h"
#include "miscellaneous/iconfactory.h"
#include "miscellaneous/mutex.h"
#include "miscellaneous/settings.h"
#include "miscellaneous/systemfactory.h"
#include "services/standard/standardfeed.h"
#include "services/standard/standardfeedsimportexportmodel.h"
#include "services/standard/standardserviceroot.h"

#include "exceptions/applicationexception.h"
#include "gui/dialogs/formdatabasecleanup.h"
#include "gui/dialogs/formmain.h"

#if defined(USE_WEBENGINE)
#include "gui/webbrowser.h"
#else
#include "gui/messagepreviewer.h"
#endif

#include <QAction>
#include <QDebug>
#include <QLineEdit>
#include <QMenu>
#include <QPointer>
#include <QProgressBar>
#include <QSplitter>
#include <QStatusBar>
#include <QToolBar>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidgetAction>

FeedMessageViewer::FeedMessageViewer(QWidget *parent) : TabContent(parent), m_toolBarsEnabled(true),
    m_listHeadersEnabled(true),
    m_toolBarFeeds(new FeedsToolBar(tr("Toolbar for feeds"), this)),
    m_toolBarMessages(new MessagesToolBar(tr("Toolbar for messages"), this)),
    m_messagesView(new MessagesView(this)), m_feedsView(new FeedsView(this)),

#if defined(USE_WEBENGINE)
    m_messagesBrowser(new WebBrowser(this))
{
#else
    m_messagesBrowser(new MessagePreviewer(this))
{
#endif

    initialize();
    initializeViews();

    //loadMessageViewerFonts();
    createConnections();
}

FeedMessageViewer::~FeedMessageViewer()
{
    qDebugNN << LOGSEC_GUI << "Destroying FeedMessageViewer instance.";
}

#if defined(USE_WEBENGINE)

WebBrowser *FeedMessageViewer::webBrowser() const
{
    return m_messagesBrowser;
}

#endif

FeedsView *FeedMessageViewer::feedsView() const
{
    return m_feedsView;
}

MessagesView *FeedMessageViewer::messagesView() const
{
    return m_messagesView;
}

MessagesToolBar *FeedMessageViewer::messagesToolBar() const
{
    return m_toolBarMessages;
}

FeedsToolBar *FeedMessageViewer::feedsToolBar() const
{
    return m_toolBarFeeds;
}

void FeedMessageViewer::saveSize()
{
    Settings *settings = qApp->settings();

    m_feedsView->saveAllExpandStates();

    // Store offsets of splitters.
    settings->setValue(GROUP(GUI), GUI::SplitterFeeds, QString(m_feedSplitter->saveState().toBase64()));
    settings->setValue(GROUP(GUI), GUI::SplitterMessages,
                       QString(m_messageSplitter->saveState().toBase64()));
    settings->setValue(GROUP(GUI), GUI::MessageViewState,
                       QString(m_messagesView->header()->saveState().toBase64()));

    // Store "visibility" of toolbars and list headers.
    settings->setValue(GROUP(GUI), GUI::ToolbarsVisible, m_toolBarsEnabled);
    settings->setValue(GROUP(GUI), GUI::ListHeadersVisible, m_listHeadersEnabled);
}

void FeedMessageViewer::loadSize()
{
    const Settings *settings = qApp->settings();

    // Restore offsets of splitters.
    m_feedSplitter->restoreState(QByteArray::fromBase64(settings->value(GROUP(GUI),
                                 SETTING(GUI::SplitterFeeds)).toString().toLocal8Bit()));
    m_messageSplitter->restoreState(QByteArray::fromBase64(settings->value(GROUP(GUI),
                                    SETTING(GUI::SplitterMessages)).toString().toLocal8Bit()));

    QString settings_msg_header = settings->value(GROUP(GUI),
                                  SETTING(GUI::MessageViewState)).toString();

    if (!settings_msg_header.isEmpty()) {
        m_messagesView->header()->restoreState(QByteArray::fromBase64(settings_msg_header.toLocal8Bit()));
    }
}

void FeedMessageViewer::loadMessageViewerFonts()
{
    m_messagesBrowser->reloadFontSettings();
    m_messagesView->reloadFontSettings();
    m_feedsView->reloadFontSettings();
}

bool FeedMessageViewer::areToolBarsEnabled() const
{
    return m_toolBarsEnabled;
}

bool FeedMessageViewer::areListHeadersEnabled() const
{
    return m_listHeadersEnabled;
}

void FeedMessageViewer::switchMessageSplitterOrientation()
{
    if (m_messageSplitter->orientation() == Qt::Vertical) {
        m_messageSplitter->setOrientation(Qt::Horizontal);
    } else {
        m_messageSplitter->setOrientation(Qt::Vertical);
    }
}

void FeedMessageViewer::setToolBarsEnabled(bool enable)
{
    m_toolBarsEnabled = enable;
    m_toolBarFeeds->setVisible(enable);
    m_toolBarMessages->setVisible(enable);
}

void FeedMessageViewer::setListHeadersEnabled(bool enable)
{
    m_listHeadersEnabled = enable;
    m_feedsView->header()->setVisible(enable);
    m_messagesView->header()->setVisible(enable);
}

void FeedMessageViewer::switchFeedComponentVisibility()
{
    auto *sen = qobject_cast<QAction *>(sender());

    if (sen != nullptr) {
        m_feedsWidget->setVisible(sen->isChecked());
    } else {
        m_feedsWidget->setVisible(!m_feedsWidget->isVisible());
    }
}

void FeedMessageViewer::toggleShowOnlyUnreadMessages()
{
    const QAction *origin = qobject_cast<QAction *>(sender());

    if (origin == nullptr) {
        m_messagesView->switchShowUnreadOnly(true, false);
    } else {
        m_messagesView->switchShowUnreadOnly(true, origin->isChecked());
    }
}

void FeedMessageViewer::toggleShowOnlyUnreadFeeds()
{
    const QAction *origin = qobject_cast<QAction *>(sender());

    if (origin == nullptr) {
        m_feedsView->model()->invalidateReadFeedsFilter(true, false);
    } else {
        m_feedsView->model()->invalidateReadFeedsFilter(true, origin->isChecked());
    }
}

void FeedMessageViewer::toggleShowFeedTreeBranches()
{
    const QAction *origin = qobject_cast<QAction *>(sender());

    m_feedsView->setRootIsDecorated(origin->isChecked());
    qApp->settings()->setValue(GROUP(Feeds), Feeds::ShowTreeBranches, origin->isChecked());
}

void FeedMessageViewer::displayMessage(const Message &message, RootItem *root)
{
    if (qApp->settings()->value(GROUP(Messages), SETTING(Messages::EnableMessagePreview)).toBool()) {
        m_messagesBrowser->loadMessage(message, root);
    } else {
        m_messagesBrowser->hide();
    }
}

void FeedMessageViewer::createConnections()
{
    // Filtering & searching.
    connect(m_toolBarMessages, &MessagesToolBar::messageSearchPatternChanged, m_messagesView,
            &MessagesView::searchMessages);
    connect(m_toolBarMessages, &MessagesToolBar::messageFilterChanged, m_messagesView,
            &MessagesView::filterMessages);

#if defined(USE_WEBENGINE)
    connect(m_messagesView, &MessagesView::currentMessageRemoved, m_messagesBrowser,
            &WebBrowser::clear);
    connect(m_messagesBrowser, &WebBrowser::markMessageRead, m_messagesView->sourceModel(),
            &MessagesModel::setMessageReadById);
    connect(m_messagesBrowser, &WebBrowser::markMessageImportant, m_messagesView->sourceModel(),
            &MessagesModel::setMessageImportantById);
#else
    connect(m_messagesView, &MessagesView::currentMessageRemoved, m_messagesBrowser,
            &MessagePreviewer::clear);
    connect(m_messagesBrowser, &MessagePreviewer::markMessageRead, m_messagesView->sourceModel(),
            &MessagesModel::setMessageReadById);
    connect(m_messagesBrowser, &MessagePreviewer::markMessageImportant,
            m_messagesView->sourceModel(), &MessagesModel::setMessageImportantById);
#endif

    connect(m_messagesView, &MessagesView::currentMessageChanged, this,
            &FeedMessageViewer::displayMessage);

    // If user selects feeds, load their messages.
    connect(m_feedsView, &FeedsView::itemSelected, m_messagesView, &MessagesView::loadItem);
    connect(m_feedsView, &FeedsView::requestViewNextUnreadMessage, m_messagesView,
            &MessagesView::selectNextUnreadItem);

    // State of many messages is changed, then we need
    // to reload selections.
    connect(m_feedsView->sourceModel(), &FeedsModel::reloadMessageListRequested, m_messagesView,
            &MessagesView::reloadSelections);
}

void FeedMessageViewer::initialize()
{
    // Initialize/populate toolbars.
    m_toolBarFeeds->setFloatable(false);
    m_toolBarFeeds->setMovable(false);
    m_toolBarFeeds->setAllowedAreas(Qt::TopToolBarArea);
    m_toolBarMessages->setFloatable(false);
    m_toolBarMessages->setMovable(false);
    m_toolBarMessages->setAllowedAreas(Qt::TopToolBarArea);
    m_messagesBrowser->clear();

    // Now refresh visual setup.
    refreshVisualProperties();
}

void FeedMessageViewer::initializeViews()
{
    m_feedsWidget = new QWidget(this);
    m_messagesWidget = new QWidget(this);
    m_feedSplitter = new QSplitter(Qt::Horizontal, this);
    m_messageSplitter = new QSplitter(Qt::Vertical, this);

    // Instantiate needed components.
    auto *central_layout = new QVBoxLayout(this);
    auto *feed_layout = new QVBoxLayout(m_feedsWidget);
    auto *message_layout = new QVBoxLayout(m_messagesWidget);

    // Set layout properties.
    central_layout->setMargin(0);
    central_layout->setSpacing(0);
    feed_layout->setMargin(0);
    feed_layout->setSpacing(0);
    message_layout->setMargin(0);
    message_layout->setSpacing(0);

    // Set views.
    m_feedsView->setFrameStyle(QFrame::NoFrame);
    m_messagesView->setFrameStyle(QFrame::NoFrame);

    // Setup message splitter.
    m_messageSplitter->setObjectName(QSL("MessageSplitter"));
    m_messageSplitter->setHandleWidth(1);
    m_messageSplitter->setOpaqueResize(false);
    m_messageSplitter->setChildrenCollapsible(false);
    m_messageSplitter->addWidget(m_messagesView);
    m_messageSplitter->addWidget(m_messagesBrowser);

    // Assemble message-related components to single widget.
    message_layout->addWidget(m_toolBarMessages);
    message_layout->addWidget(m_messageSplitter);

    // Assemble feed-related components to another widget.
    feed_layout->addWidget(m_toolBarFeeds);
    feed_layout->addWidget(m_feedsView);

    // Assembler everything together.
    m_feedSplitter->setHandleWidth(1);
    m_feedSplitter->setOpaqueResize(false);
    m_feedSplitter->setChildrenCollapsible(false);
    m_feedSplitter->addWidget(m_feedsWidget);
    m_feedSplitter->addWidget(m_messagesWidget);

    // Add toolbar and main feeds/messages widget to main layout.
    central_layout->addWidget(m_feedSplitter);
    setTabOrder(m_feedsView, m_messagesView);
    setTabOrder(m_messagesView, m_toolBarFeeds);
    setTabOrder(m_toolBarFeeds, m_toolBarMessages);
    setTabOrder(m_toolBarMessages, m_messagesBrowser);

    // Set initial ratio of sizes.
    m_feedSplitter->setStretchFactor(0, 1);
    m_feedSplitter->setStretchFactor(1, 3);
}

void FeedMessageViewer::refreshVisualProperties()
{
    const Qt::ToolButtonStyle button_style =
        static_cast<Qt::ToolButtonStyle>(qApp->settings()->value(GROUP(GUI),
                                         SETTING(GUI::ToolbarStyle)).toInt());

    m_toolBarFeeds->setToolButtonStyle(button_style);
    m_toolBarMessages->setToolButtonStyle(button_style);
}
