// For license of this file, see <project-root-folder>/LICENSE.md.

#include "gui/dialogs/formrestoredatabasesettings.h"

#include "exceptions/applicationexception.h"
#include "gui/dialogs/formmain.h"
#include "gui/messagebox.h"
#include "miscellaneous/iconfactory.h"

#include "QFileDialog"

FormRestoreDatabaseSettings::FormRestoreDatabaseSettings(QWidget &parent) : QDialog(&parent),
    m_shouldRestart(false)
{
    m_ui.setupUi(this);
    m_btnRestart = m_ui.m_buttonBox->addButton(tr("Restart"), QDialogButtonBox::ActionRole);
    m_ui.m_lblResult->setStatus(WidgetWithStatus::StatusType::Warning, tr("No operation executed yet."),
                                tr("No operation executed yet."));
    setWindowIcon(qApp->icons()->fromTheme(QSL("document-import")));
    setWindowFlags(Qt::MSWindowsFixedSizeDialogHint | Qt::Dialog | Qt::WindowSystemMenuHint);
    connect(m_btnRestart, &QPushButton::clicked, this, [ = ]() {
        m_shouldRestart = true;
        close();
    });
    connect(m_ui.m_btnSelectFolder, SIGNAL(clicked()), this, SLOT(selectFolder()));
    connect(m_ui.m_groupDatabase, SIGNAL(toggled(bool)), this, SLOT(checkOkButton()));
    connect(m_ui.m_groupSettings, SIGNAL(toggled(bool)), this, SLOT(checkOkButton()));
    connect(m_ui.m_buttonBox->button(QDialogButtonBox::Ok), SIGNAL(clicked()), this,
            SLOT(performRestoration()));
    selectFolder(qApp->documentsFolder());
}

FormRestoreDatabaseSettings::~FormRestoreDatabaseSettings()
{
    qDebug("Destroying FormRestoreDatabaseSettings instance.");
}

void FormRestoreDatabaseSettings::performRestoration()
{
    m_ui.m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

    try {
        qApp->restoreDatabaseSettings(m_ui.m_groupDatabase->isChecked(),
                                      m_ui.m_groupSettings->isChecked(),
                                      m_ui.m_listDatabase->currentRow() >= 0 ? m_ui.m_listDatabase->currentItem()->data(
                                          Qt::UserRole).toString() : QString(),
                                      m_ui.m_listSettings->currentRow() >= 0 ? m_ui.m_listSettings->currentItem()->data(
                                          Qt::UserRole).toString() : QString());
        m_btnRestart->setEnabled(true);
        m_ui.m_lblResult->setStatus(WidgetWithStatus::StatusType::Ok,
                                    tr("Restoration was initiated. Restart to proceed."),
                                    tr("You need to restart application for restoration process to finish."));
    } catch (const ApplicationException &ex) {
        m_ui.m_lblResult->setStatus(WidgetWithStatus::StatusType::Error, ex.message(),
                                    tr("Database and/or settings were not copied to restoration directory successully."));
    }
}

void FormRestoreDatabaseSettings::checkOkButton()
{
    m_btnRestart->setEnabled(false);
    m_ui.m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(
        !m_ui.m_lblSelectFolder->label()->text().isEmpty() &&
        ((m_ui.m_groupDatabase->isChecked() &&
          m_ui.m_listDatabase->currentRow() >= 0) ||
         (m_ui.m_groupSettings->isChecked() &&
          m_ui.m_listSettings->currentRow() >= 0)));
}

void FormRestoreDatabaseSettings::selectFolderWithGui()
{
    selectFolder();
}

void FormRestoreDatabaseSettings::selectFolder(QString folder)
{
    if (folder.isEmpty()) {
        folder = QFileDialog::getExistingDirectory(this, tr("Select source directory"),
                 m_ui.m_lblSelectFolder->label()->text());
    }

    if (!folder.isEmpty()) {
        m_ui.m_lblSelectFolder->setStatus(WidgetWithStatus::StatusType::Ok,
                                          QDir::toNativeSeparators(folder),
                                          tr("Good source directory is specified."));
    } else {
        return;
    }

    const QDir selected_folder(folder);
    const QFileInfoList available_databases = selected_folder.entryInfoList(QStringList() << QString(
                "*") + BACKUP_SUFFIX_DATABASE,
            QDir::Files | QDir::NoDotAndDotDot | QDir::Readable | QDir::CaseSensitive | QDir::NoSymLinks,
            QDir::Name);
    const QFileInfoList available_settings = selected_folder.entryInfoList(QStringList() << QString(
                "*") + BACKUP_SUFFIX_SETTINGS,
            QDir::Files | QDir::NoDotAndDotDot | QDir::Readable | QDir::CaseSensitive | QDir::NoSymLinks,
            QDir::Name);

    m_ui.m_listDatabase->clear();
    m_ui.m_listSettings->clear();

    for (const QFileInfo &database_file : available_databases) {
        QListWidgetItem *database_item = new QListWidgetItem(database_file.fileName(), m_ui.m_listDatabase);

        database_item->setData(Qt::UserRole, database_file.absoluteFilePath());
        database_item->setToolTip(QDir::toNativeSeparators(database_file.absoluteFilePath()));
    }

    for (const QFileInfo &settings_file : available_settings) {
        QListWidgetItem *settings_item = new QListWidgetItem(settings_file.fileName(), m_ui.m_listSettings);

        settings_item->setData(Qt::UserRole, settings_file.absoluteFilePath());
        settings_item->setToolTip(QDir::toNativeSeparators(settings_file.absoluteFilePath()));
    }

    if (!available_databases.isEmpty()) {
        m_ui.m_listDatabase->setCurrentRow(0);
    }

    if (!available_settings.isEmpty()) {
        m_ui.m_listSettings->setCurrentRow(0);
    }

    m_ui.m_groupDatabase->setChecked(!available_databases.isEmpty());
    m_ui.m_groupSettings->setChecked(!available_settings.isEmpty());
}
