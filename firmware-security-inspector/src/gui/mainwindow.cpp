#include "mainwindow.h"
#include "widgets/dashboard_widget.h"
#include "widgets/cpu_widget.h"
#include "widgets/msr_widget.h"
#include "widgets/acpi_widget.h"
#include "widgets/security_audit_widget.h"
#include "widgets/developer_widget.h"
#include "widgets/hex_viewer.h"
#include "widgets/firmware_widget.h"
#include "widgets/secureboot_widget.h"
#include "widgets/tpm_widget.h"
#include "widgets/smbios_widget.h"
#include "widgets/pcie_widget.h"
#include "widgets/memory_widget.h"
#include "widgets/registers_widget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QFileDialog>
#include <QMessageBox>
#include <QLabel>
#include <QGroupBox>
#include <QTableWidget>
#include <QTreeWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QHeaderView>
#include <QApplication>
#include <QSplitter>
#include <QProgressBar>
#include <QStackedWidget>
#include <QListWidget>
#include <unistd.h> 

#ifdef _WIN32
#include <windows.h>
#endif
#include <time.h>
#include <cstring>

/* ===== DataCollectorThread ===== */
DataCollectorThread::DataCollectorThread(QObject *parent)
    : QThread(parent)
{
    memset(&m_snap, 0, sizeof(m_snap));
}

void DataCollectorThread::run()
{
    emit progressUpdate(5,  "Collecting CPU info...");
    fsi_cpu_collect(&m_snap.cpu);

    emit progressUpdate(15, "Collecting firmware info...");
    fsi_firmware_collect(&m_snap.firmware);

    emit progressUpdate(25, "Collecting Secure Boot status...");
    fsi_secureboot_collect(&m_snap.secureboot);

    emit progressUpdate(35, "Collecting TPM info...");
    fsi_tpm_collect(&m_snap.tpm);

    emit progressUpdate(45, "Collecting ACPI tables...");
    fsi_acpi_collect(&m_snap.acpi);

    emit progressUpdate(55, "Collecting SMBIOS info...");
    fsi_smbios_collect(&m_snap.smbios);

    emit progressUpdate(65, "Enumerating PCIe devices...");
    fsi_pcie_collect(&m_snap.pcie);

    emit progressUpdate(75, "Reading memory map...");
    fsi_memory_collect(&m_snap.memory);

    emit progressUpdate(85, "Reading MSRs...");
    fsi_msr_collect(&m_snap.msr);

    emit progressUpdate(90, "Reading registers...");
    fsi_registers_collect(&m_snap.registers);

    emit progressUpdate(95, "Running security audit...");
    fsi_audit_run(&m_snap.audit,
                  &m_snap.cpu,
                  &m_snap.secureboot,
                  &m_snap.tpm,
                  &m_snap.registers,
                  &m_snap.msr);

    /* Timestamp e hostname */
    time_t now = time(nullptr);
    struct tm *tm_info = localtime(&now);
    if (tm_info) {
        strftime(m_snap.timestamp, sizeof(m_snap.timestamp),
                 "%Y-%m-%dT%H:%M:%S", tm_info);
    } else {
        strcpy(m_snap.timestamp, "1970-01-01T00:00:00");
    }

    /* Hostname con fallback per Windows */
#ifdef _WIN32
    DWORD size = sizeof(m_snap.hostname);
    if (GetComputerNameA(m_snap.hostname, &size) == 0) {
        strcpy(m_snap.hostname, "Windows-Host");
    }
#else
    gethostname(m_snap.hostname, sizeof(m_snap.hostname) - 1);
#endif

    emit progressUpdate(100, "Done.");
    emit collectionDone();
}


static QWidget *makePlaceholder(const QString &title)
{
    QWidget *w = new QWidget;
    QVBoxLayout *l = new QVBoxLayout(w);
    QLabel *lbl = new QLabel(title);
    lbl->setStyleSheet("color: #569cd6; font-size: 14px; font-weight: bold;");
    l->addWidget(lbl);
    l->addStretch();
    return w;
}


static QTableWidget *makeInfoTable(QWidget *parent)
{
    QTableWidget *t = new QTableWidget(0, 2, parent);
    t->setHorizontalHeaderLabels({"Field", "Value"});
    t->horizontalHeader()->setStretchLastSection(true);
    t->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    t->verticalHeader()->hide();
    t->setEditTriggers(QAbstractItemView::NoEditTriggers);
    t->setAlternatingRowColors(true);
    t->setSelectionBehavior(QAbstractItemView::SelectRows);
    t->setShowGrid(false);
    t->setFont(QFont("Consolas", 9));
    return t;
}

static void tableAddRow(QTableWidget *t,
                        const QString &field, const QString &value,
                        const QColor &valueColor)
{
    int row = t->rowCount();
    t->insertRow(row);
    t->setItem(row, 0, new QTableWidgetItem(field));

    QTableWidgetItem *valItem = new QTableWidgetItem(value);
    if (valueColor.isValid())
        valItem->setForeground(valueColor);
    t->setItem(row, 1, valItem);
}

/* ===== MainWindow ===== */
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_collector(nullptr)
    , m_snapshot(nullptr)
    , m_dataReady(false)
{
    setupMenuBar();
    setupLayout();
    setupNavBar();
    setupPages();


    onRefreshClicked();
}

MainWindow::~MainWindow()
{
    if (m_collector) {
        m_collector->wait();
        delete m_collector;
    }
    if (m_snapshot) {
        fsi_system_snapshot_free(m_snapshot);
        delete m_snapshot;
    }
}

void MainWindow::setupMenuBar()
{
    QMenuBar *mb = menuBar();

    QMenu *fileMenu = mb->addMenu("&File");
    fileMenu->addAction("&Refresh", QKeySequence::Refresh,
                        this, &MainWindow::onRefreshClicked);
    fileMenu->addSeparator();
    fileMenu->addAction("Save &Snapshot", this, &MainWindow::onSaveSnapshot);
    fileMenu->addAction("&Load Snapshot", this, &MainWindow::onLoadSnapshot);
    fileMenu->addAction("&Compare Snapshots", this,
                        &MainWindow::onCompareSnapshots);
    fileMenu->addSeparator();
    fileMenu->addAction("E&xport Report...", this, &MainWindow::onExportReport);
    fileMenu->addSeparator();
    fileMenu->addAction("&Quit", QKeySequence::Quit,
                        qApp, &QApplication::quit);

    QMenu *helpMenu = mb->addMenu("&Help");
    helpMenu->addAction("&About", this, [this]() {
        QMessageBox::about(this, "About FSI",
            "<b>Firmware Security Inspector v1.0</b><br>"
            "Read-only firmware and security analysis tool.<br><br>"
            "Written in C17, x86-64 NASM Assembly, C++ Qt6.<br>"
            "Open source. Never modifies firmware or hardware.");
    });
}

void MainWindow::setupLayout()
{
    QWidget *central = new QWidget(this);
    setCentralWidget(central);

    QVBoxLayout *mainLayout = new QVBoxLayout(central);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    /* Toolbar area */
    QWidget *toolbar = new QWidget;
    toolbar->setFixedHeight(36);
    toolbar->setStyleSheet("background: #2d2d30; border-bottom: 1px solid #3f3f46;");
    QHBoxLayout *tbLayout = new QHBoxLayout(toolbar);
    tbLayout->setContentsMargins(8, 4, 8, 4);

    QPushButton *refreshBtn = new QPushButton("⟳ Refresh");
    refreshBtn->setFixedWidth(90);
    refreshBtn->setStyleSheet(
        "QPushButton{background:#37373d;color:#d4d4d4;"
        "border:1px solid #555;padding:3px 8px;}"
        "QPushButton:hover{background:#4a4a52;}");
    connect(refreshBtn, &QPushButton::clicked,
            this, &MainWindow::onRefreshClicked);
    tbLayout->addWidget(refreshBtn);

    QPushButton *snapBtn = new QPushButton("📷 Save Snapshot");
    snapBtn->setFixedWidth(130);
    snapBtn->setStyleSheet(refreshBtn->styleSheet());
    connect(snapBtn, &QPushButton::clicked, this, &MainWindow::onSaveSnapshot);
    tbLayout->addWidget(snapBtn);

    QPushButton *reportBtn = new QPushButton("📄 Export Report");
    reportBtn->setFixedWidth(130);
    reportBtn->setStyleSheet(refreshBtn->styleSheet());
    connect(reportBtn, &QPushButton::clicked, this, &MainWindow::onExportReport);
    tbLayout->addWidget(reportBtn);

    tbLayout->addStretch();

    m_statusLabel = new QLabel("Ready");
    m_statusLabel->setStyleSheet("color: #858585; font-size: 11px;");
    tbLayout->addWidget(m_statusLabel);

    m_progressBar = new QProgressBar;
    m_progressBar->setFixedWidth(150);
    m_progressBar->setFixedHeight(14);
    m_progressBar->setTextVisible(false);
    m_progressBar->hide();
    tbLayout->addWidget(m_progressBar);

    mainLayout->addWidget(toolbar);

    /* Splitter: sidebar | main panel */
    m_splitter = new QSplitter(Qt::Horizontal);
    m_splitter->setHandleWidth(1);
    mainLayout->addWidget(m_splitter, 1);

    /* Nav list */
    m_navList = new QListWidget;
    m_navList->setFixedWidth(160);
    m_navList->setStyleSheet(
        "QListWidget{background:#252526;border:none;color:#cccccc;"
        "font-family:Consolas;font-size:12px;}"
        "QListWidget::item{padding:6px 12px;border-bottom:1px solid #2d2d30;}"
        "QListWidget::item:selected{background:#094771;color:#fff;}"
        "QListWidget::item:hover{background:#2a2d2e;}");
    m_splitter->addWidget(m_navList);

    /* Main stack */
    m_stack = new QStackedWidget;
    m_stack->setStyleSheet("background: #1e1e1e;");
    m_splitter->addWidget(m_stack);

    m_splitter->setStretchFactor(0, 0);
    m_splitter->setStretchFactor(1, 1);
}

void MainWindow::setupNavBar()
{
    const QStringList navItems = {
        "Dashboard",
        "Firmware",
        "CPU",
        "Registers",
        "MSRs",
        "Secure Boot",
        "TPM",
        "ACPI",
        "SMBIOS",
        "PCIe",
        "Memory",
        "Security Audit",
        "Reports",
        "Developer Mode"
    };

    for (const QString &item : navItems) {
        m_navList->addItem(item);
    }

    connect(m_navList, &QListWidget::itemClicked,
            this, &MainWindow::onNavItemSelected);
    m_navList->setCurrentRow(0);
}

void MainWindow::setupPages()
{
    /* Dashboard */
    m_pageDashboard  = new DashboardWidget(this);
    m_pageFirmware   = new FirmwareWidget(this);
    m_pageCpu        = new CpuWidget(this);
    m_pageRegisters  = new RegistersWidget(this);
    m_pageMsr        = new MsrWidget(this);
    m_pageSecureBoot = new SecureBootWidget(this);
    m_pageTpm        = new TpmWidget(this);
    m_pageAcpi       = new AcpiWidget(this);
    m_pageSmbios     = new SmbiosWidget(this);
    m_pagePcie       = new PcieWidget(this);
    m_pageMemory     = new MemoryWidget(this);
    m_pageAudit      = new SecurityAuditWidget(this);
    m_pageReports    = makePlaceholder("Reports");
    m_pageDeveloper  = new DeveloperWidget(this);

    m_stack->addWidget(m_pageDashboard);   /* 0 */
    m_stack->addWidget(m_pageFirmware);    /* 1 */
    m_stack->addWidget(m_pageCpu);         /* 2 */
    m_stack->addWidget(m_pageRegisters);   /* 3 */
    m_stack->addWidget(m_pageMsr);         /* 4 */
    m_stack->addWidget(m_pageSecureBoot);  /* 5 */
    m_stack->addWidget(m_pageTpm);         /* 6 */
    m_stack->addWidget(m_pageAcpi);        /* 7 */
    m_stack->addWidget(m_pageSmbios);      /* 8 */
    m_stack->addWidget(m_pagePcie);        /* 9 */
    m_stack->addWidget(m_pageMemory);      /* 10 */
    m_stack->addWidget(m_pageAudit);       /* 11 */
    m_stack->addWidget(m_pageReports);     /* 12 */
    m_stack->addWidget(m_pageDeveloper);   /* 13 */
}

void MainWindow::onNavItemSelected(QListWidgetItem *item)
{
    int row = m_navList->row(item);
    m_stack->setCurrentIndex(row);
}

void MainWindow::onRefreshClicked()
{
    if (m_collector && m_collector->isRunning()) return;

    if (m_snapshot) {
        fsi_system_snapshot_free(m_snapshot);
        delete m_snapshot;
        m_snapshot = nullptr;
    }

    m_dataReady = false;
    m_progressBar->show();
    m_progressBar->setValue(0);
    setStatus("Collecting data...");

    m_collector = new DataCollectorThread(this);
    connect(m_collector, &DataCollectorThread::progressUpdate,
            this, &MainWindow::onProgressUpdate);
    connect(m_collector, &DataCollectorThread::collectionDone,
            this, &MainWindow::onCollectionDone);
    m_collector->start();
}

void MainWindow::onProgressUpdate(int percent, const QString &msg)
{
    m_progressBar->setValue(percent);
    setStatus(msg);
}

void MainWindow::onCollectionDone()
{
    m_snapshot   = m_collector->snapshot();
    m_dataReady  = true;
    m_progressBar->hide();
    setStatus(QString("Data collected at %1")
              .arg(QString::fromLocal8Bit(m_snapshot->timestamp)));
    updateAllPages();
}

void MainWindow::updateAllPages()
{
    if (!m_snapshot || !m_dataReady) return;

    /* Aggiorna ogni widget con i nuovi dati */
    if (auto *dw = qobject_cast<DashboardWidget*>(m_pageDashboard))
        dw->updateData(m_snapshot);

    if (auto *fw = qobject_cast<FirmwareWidget*>(m_pageFirmware))
        fw->updateData(&m_snapshot->firmware);

    if (auto *cw = qobject_cast<CpuWidget*>(m_pageCpu))
        cw->updateData(&m_snapshot->cpu);

    if (auto *rw = qobject_cast<RegistersWidget*>(m_pageRegisters))
        rw->updateData(&m_snapshot->registers);

    if (auto *mw = qobject_cast<MsrWidget*>(m_pageMsr))
        mw->updateData(&m_snapshot->msr);

    if (auto *sbw = qobject_cast<SecureBootWidget*>(m_pageSecureBoot))
        sbw->updateData(&m_snapshot->secureboot);

    if (auto *tw = qobject_cast<TpmWidget*>(m_pageTpm))
        tw->updateData(&m_snapshot->tpm);

    if (auto *aw = qobject_cast<AcpiWidget*>(m_pageAcpi))
        aw->updateData(&m_snapshot->acpi);

    if (auto *sw = qobject_cast<SmbiosWidget*>(m_pageSmbios))
        sw->updateData(&m_snapshot->smbios);

    if (auto *pw = qobject_cast<PcieWidget*>(m_pagePcie))
        pw->updateData(&m_snapshot->pcie);

    if (auto *mw2 = qobject_cast<MemoryWidget*>(m_pageMemory))
        mw2->updateData(&m_snapshot->memory);

    if (auto *saw = qobject_cast<SecurityAuditWidget*>(m_pageAudit))
        saw->updateData(&m_snapshot->audit);

    if (auto *dv = qobject_cast<DeveloperWidget*>(m_pageDeveloper))
        dv->updateData(m_snapshot);
}

void MainWindow::onSaveSnapshot()
{
    if (!m_dataReady || !m_snapshot) {
        QMessageBox::warning(this, "No Data",
                             "No data collected yet. Click Refresh first.");
        return;
    }
    QString path = QFileDialog::getSaveFileName(
        this, "Save Snapshot", "",
        "FSI Snapshot (*.fsi);;All Files (*)");
    if (path.isEmpty()) return;

    if (fsi_snapshot_save(m_snapshot, path.toLocal8Bit().constData()) == 0) {
        setStatus("Snapshot saved to " + path);
    } else {
        QMessageBox::critical(this, "Error",
                              "Failed to save snapshot to " + path);
    }
}

void MainWindow::onLoadSnapshot()
{
    QString path = QFileDialog::getOpenFileName(
        this, "Load Snapshot", "",
        "FSI Snapshot (*.fsi);;All Files (*)");
    if (path.isEmpty()) return;

    fsi_system_snapshot_t *snap = new fsi_system_snapshot_t;
    memset(snap, 0, sizeof(*snap));

    if (fsi_snapshot_load(snap, path.toLocal8Bit().constData()) != 0) {
        QMessageBox::critical(this, "Error",
                              "Failed to load snapshot from " + path);
        delete snap;
        return;
    }

    if (m_snapshot) {
        fsi_system_snapshot_free(m_snapshot);
        delete m_snapshot;
    }
    fsi_snapshot_free_loaded(snap);
    m_snapshot  = snap;
    m_dataReady = true;
    setStatus("Loaded snapshot: " +
              QString::fromLocal8Bit(snap->timestamp));
    updateAllPages();
}

void MainWindow::onCompareSnapshots()
{
    QString pathA = QFileDialog::getOpenFileName(
        this, "Select Snapshot A", "",
        "FSI Snapshot (*.fsi);;All Files (*)");
    if (pathA.isEmpty()) return;

    QString pathB = QFileDialog::getOpenFileName(
        this, "Select Snapshot B", "",
        "FSI Snapshot (*.fsi);;All Files (*)");
    if (pathB.isEmpty()) return;

    fsi_system_snapshot_t a = {0}, b = {0};
    if (fsi_snapshot_load(&a, pathA.toLocal8Bit().constData()) != 0 ||
        fsi_snapshot_load(&b, pathB.toLocal8Bit().constData()) != 0) {
        QMessageBox::critical(this, "Error", "Failed to load snapshots.");
        return;
    }


    QWidget *diffWin = new QWidget(nullptr, Qt::Window);
    diffWin->setWindowTitle("Snapshot Comparison");
    diffWin->resize(700, 500);
    QVBoxLayout *l = new QVBoxLayout(diffWin);
    QTextEdit *te = new QTextEdit;
    te->setReadOnly(true);
    te->setFont(QFont("Consolas", 9));
    l->addWidget(te);


    QString diff;
    diff += QString("Snapshot A: %1 @ %2\n")
            .arg(QString::fromLocal8Bit(a.hostname))
            .arg(QString::fromLocal8Bit(a.timestamp));
    diff += QString("Snapshot B: %1 @ %2\n\n")
            .arg(QString::fromLocal8Bit(b.hostname))
            .arg(QString::fromLocal8Bit(b.timestamp));

#define QDIFF_STR(sec, field, fa, fb) \
    if (strcmp(fa, fb) != 0) \
        diff += QString("[CHANGED] %1.%2:\n  A: %3\n  B: %4\n") \
                .arg(sec).arg(field) \
                .arg(QString::fromLocal8Bit(fa)) \
                .arg(QString::fromLocal8Bit(fb));

#define QDIFF_INT(sec, field, fa, fb) \
    if ((fa) != (fb)) \
        diff += QString("[CHANGED] %1.%2: %3 -> %4\n") \
                .arg(sec).arg(field).arg(fa).arg(fb);

#define QDIFF_BOOL(sec, field, fa, fb) \
    if ((fa) != (fb)) \
        diff += QString("[CHANGED] %1.%2: %3 -> %4\n") \
                .arg(sec).arg(field) \
                .arg((fa) ? "YES" : "NO") \
                .arg((fb) ? "YES" : "NO");

    QDIFF_INT ("Security",   "score",        a.audit.score, b.audit.score)
    QDIFF_STR ("SecureBoot", "state",
               a.secureboot.state_str, b.secureboot.state_str)
    QDIFF_BOOL("SecureBoot", "dbx",
               a.secureboot.dbx_present, b.secureboot.dbx_present)
    QDIFF_STR ("Firmware",   "bios_version",
               a.firmware.bios_version, b.firmware.bios_version)
    QDIFF_STR ("TPM",        "version",
               a.tpm.version_str, b.tpm.version_str)
    QDIFF_BOOL("CPU",        "smep",  a.cpu.features.smep,  b.cpu.features.smep)
    QDIFF_BOOL("CPU",        "smap",  a.cpu.features.smap,  b.cpu.features.smap)
    QDIFF_BOOL("CPU",        "cet_ss",a.cpu.features.cet_ss,b.cpu.features.cet_ss)
    QDIFF_INT ("ACPI",       "tables",a.acpi.count, b.acpi.count)
    QDIFF_INT ("PCIe",       "devices",a.pcie.count,b.pcie.count)

#undef QDIFF_STR
#undef QDIFF_INT
#undef QDIFF_BOOL

    if (!diff.contains("[CHANGED]"))
        diff += "No differences found between snapshots.\n";

    te->setPlainText(diff);
    diffWin->show();

    fsi_snapshot_free_loaded(&a);
    fsi_snapshot_free_loaded(&b);
}

void MainWindow::onExportReport()
{
    if (!m_dataReady || !m_snapshot) {
        QMessageBox::warning(this, "No Data",
                             "No data collected yet. Click Refresh first.");
        return;
    }

    QString filter =
        "HTML Report (*.html);;"
        "JSON Report (*.json);;"
        "Text Report (*.txt)";

    QString selectedFilter;
    QString path = QFileDialog::getSaveFileName(
        this, "Export Report", "fsi_report",
        filter, &selectedFilter);
    if (path.isEmpty()) return;

    fsi_report_format_t fmt = REPORT_FORMAT_HTML;
    if (selectedFilter.contains("JSON")) fmt = REPORT_FORMAT_JSON;
    else if (selectedFilter.contains("Text")) fmt = REPORT_FORMAT_TXT;

    if (fsi_report_generate(m_snapshot, fmt,
                            path.toLocal8Bit().constData()) == 0) {
        setStatus("Report exported to " + path);
        QMessageBox::information(this, "Done",
                                 "Report saved to:\n" + path);
    } else {
        QMessageBox::critical(this, "Error",
                              "Failed to write report to " + path);
    }
}

void MainWindow::setStatus(const QString &msg)
{
    m_statusLabel->setText(msg);
}