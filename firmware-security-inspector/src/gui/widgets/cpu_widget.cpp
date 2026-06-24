#include "cpu_widget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QFont>
#include <QGroupBox>
#include <QLabel>

CpuWidget::CpuWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void CpuWidget::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(12, 12, 12, 12);
    mainLayout->setSpacing(8);

    // Header
    QLabel *title = new QLabel("CPU Information");
    title->setStyleSheet(
        "color:#569cd6;font-size:16px;font-weight:bold;"
        "border-bottom:1px solid #3f3f46;padding-bottom:4px;");
    mainLayout->addWidget(title);

    // Summary row
    QHBoxLayout *summaryLayout = new QHBoxLayout;
    m_vendorLabel = new QLabel("Vendor: --");
    m_vendorLabel->setStyleSheet("color:#9cdcfe;font-size:13px;");
    m_brandLabel = new QLabel("Brand: --");
    m_brandLabel->setStyleSheet("color:#d4d4d4;font-size:13px;");
    m_microarchLabel = new QLabel("Microarch: --");
    m_microarchLabel->setStyleSheet("color:#4ec9b0;font-size:13px;");

    summaryLayout->addWidget(m_vendorLabel);
    summaryLayout->addWidget(m_brandLabel);
    summaryLayout->addWidget(m_microarchLabel);
    summaryLayout->addStretch();
    mainLayout->addLayout(summaryLayout);

    // Tab widget
    m_tabWidget = new QTabWidget;
    m_tabWidget->setStyleSheet(
        "QTabWidget::pane{background:#1e1e1e;border:1px solid #3f3f46;}"
        "QTabBar::tab{background:#2d2d30;color:#d4d4d4;padding:6px 14px;"
        "border:1px solid #3f3f46;border-bottom:none;}"
        "QTabBar::tab:selected{background:#094771;color:#ffffff;}"
        "QTabBar::tab:hover{background:#3a3a40;}");

    // Create tabs
    m_generalTable = new QTableWidget(0, 2);
    m_generalTable->setHorizontalHeaderLabels({"Field", "Value"});
    m_generalTable->horizontalHeader()->setStretchLastSection(true);
    m_generalTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_generalTable->verticalHeader()->hide();
    m_generalTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_generalTable->setAlternatingRowColors(true);
    m_generalTable->setShowGrid(false);
    m_generalTable->setFont(QFont("Consolas", 9));

    m_featuresTable = new QTableWidget(0, 3);
    m_featuresTable->setHorizontalHeaderLabels({"Feature", "Supported", "Description"});
    m_featuresTable->horizontalHeader()->setStretchLastSection(true);
    m_featuresTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_featuresTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_featuresTable->verticalHeader()->hide();
    m_featuresTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_featuresTable->setAlternatingRowColors(true);
    m_featuresTable->setShowGrid(false);
    m_featuresTable->setFont(QFont("Consolas", 9));

    m_cacheTable = new QTableWidget(0, 5);
    m_cacheTable->setHorizontalHeaderLabels({"Level", "Type", "Size (KB)", "Ways", "Line Size"});
    m_cacheTable->horizontalHeader()->setStretchLastSection(true);
    m_cacheTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_cacheTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_cacheTable->verticalHeader()->hide();
    m_cacheTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_cacheTable->setAlternatingRowColors(true);
    m_cacheTable->setShowGrid(false);
    m_cacheTable->setFont(QFont("Consolas", 9));

    m_rawTable = new QTableWidget(0, 5);
    m_rawTable->setHorizontalHeaderLabels({"Leaf", "EAX", "EBX", "ECX", "EDX"});
    m_rawTable->horizontalHeader()->setStretchLastSection(true);
    m_rawTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_rawTable->verticalHeader()->hide();
    m_rawTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_rawTable->setAlternatingRowColors(true);
    m_rawTable->setShowGrid(false);
    m_rawTable->setFont(QFont("Consolas", 9));

    m_tabWidget->addTab(m_generalTable, "General");
    m_tabWidget->addTab(m_featuresTable, "Security Features");
    m_tabWidget->addTab(m_cacheTable, "Cache");
    m_tabWidget->addTab(m_rawTable, "Raw CPUID");

    mainLayout->addWidget(m_tabWidget);
}

void CpuWidget::updateData(const fsi_cpu_info_t *cpu)
{
    if (!cpu) return;

    m_vendorLabel->setText(QString("Vendor: %1").arg(QString::fromLocal8Bit(cpu->vendor)));
    m_brandLabel->setText(QString("Brand: %1").arg(QString::fromLocal8Bit(cpu->brand)));
    m_microarchLabel->setText(QString("Microarch: %1").arg(QString::fromLocal8Bit(cpu->microarch)));

    populateGeneralTab(cpu);
    populateFeaturesTab(cpu);
    populateCacheTab(cpu);
    populateRawTab(cpu);
}

void CpuWidget::populateGeneralTab(const fsi_cpu_info_t *cpu)
{
    m_generalTable->setRowCount(0);
    auto addRow = [this](const QString &field, const QString &value) {
        int row = m_generalTable->rowCount();
        m_generalTable->insertRow(row);
        m_generalTable->setItem(row, 0, new QTableWidgetItem(field));
        m_generalTable->setItem(row, 1, new QTableWidgetItem(value));
    };

    addRow("Vendor", QString::fromLocal8Bit(cpu->vendor));
    addRow("Brand", QString::fromLocal8Bit(cpu->brand));
    addRow("Microarchitecture", QString::fromLocal8Bit(cpu->microarch));
    addRow("Family", QString("0x%1 (effective 0x%2)").arg(cpu->family, 0, 16).arg(cpu->effective_family, 0, 16));
    addRow("Model", QString("0x%1 (effective 0x%2)").arg(cpu->model, 0, 16).arg(cpu->effective_model, 0, 16));
    addRow("Stepping", QString::number(cpu->stepping));
    addRow("Physical Cores", QString::number(cpu->physical_cores));
    addRow("Logical Cores", QString::number(cpu->logical_cores));
    addRow("Cache Levels", QString::number(cpu->cache_count));
}

void CpuWidget::populateFeaturesTab(const fsi_cpu_info_t *cpu)
{
    m_featuresTable->setRowCount(0);

    struct FeatureDef {
        const char *name;
        bool value;
        const char *desc;
    };

    FeatureDef features[] = {
        {"NX/XD", cpu->features.nx, "No-Execute / Execute Disable"},
        {"SMEP", cpu->features.smep, "Supervisor Mode Execution Prevention"},
        {"SMAP", cpu->features.smap, "Supervisor Mode Access Prevention"},
        {"UMIP", cpu->features.umip, "User-Mode Instruction Prevention"},
        {"CET-SS", cpu->features.cet_ss, "Control-flow Enforcement Shadow Stack"},
        {"CET-IBT", cpu->features.cet_ibt, "Control-flow Enforcement IBT"},
        {"AES-NI", cpu->features.aes_ni, "AES New Instructions"},
        {"VT-x", cpu->features.vtx, "Intel Virtualization Technology"},
        {"AMD-V", cpu->features.amd_v, "AMD Virtualization"},
        {"SGX", cpu->features.sgx, "Software Guard Extensions"},
        {"TXT", cpu->features.txt, "Trusted Execution Technology"},
        {"AVX", cpu->features.avx, "Advanced Vector Extensions"},
        {"AVX2", cpu->features.avx2, "Advanced Vector Extensions 2"},
        {"AVX-512", cpu->features.avx512f, "AVX-512 Foundation"},
        {"RDRAND", cpu->features.rdrand, "Hardware Random Number Generator"},
        {"RDSEED", cpu->features.rdseed, "Hardware Seed Generator"},
    };

    for (const auto &f : features) {
        int row = m_featuresTable->rowCount();
        m_featuresTable->insertRow(row);
        m_featuresTable->setItem(row, 0, new QTableWidgetItem(f.name));
        QTableWidgetItem *valItem = new QTableWidgetItem(f.value ? "YES" : "NO");
        valItem->setForeground(f.value ? QColor(78, 201, 176) : QColor(244, 71, 71));
        valItem->setFont(QFont("Consolas", 9, QFont::Bold));
        m_featuresTable->setItem(row, 1, valItem);
        m_featuresTable->setItem(row, 2, new QTableWidgetItem(f.desc));
    }
}

void CpuWidget::populateCacheTab(const fsi_cpu_info_t *cpu)
{
    m_cacheTable->setRowCount(0);
    for (int i = 0; i < cpu->cache_count; i++) {
        const fsi_cache_t *c = &cpu->caches[i];
        int row = m_cacheTable->rowCount();
        m_cacheTable->insertRow(row);
        m_cacheTable->setItem(row, 0, new QTableWidgetItem(QString("L%1").arg(c->level)));
        m_cacheTable->setItem(row, 1, new QTableWidgetItem(QString::fromLocal8Bit(c->type)));
        m_cacheTable->setItem(row, 2, new QTableWidgetItem(QString::number(c->size_kb)));
        m_cacheTable->setItem(row, 3, new QTableWidgetItem(QString::number(c->ways)));
        m_cacheTable->setItem(row, 4, new QTableWidgetItem(QString::number(c->line_size)));
    }
}

void CpuWidget::populateRawTab(const fsi_cpu_info_t *cpu)
{
    m_rawTable->setRowCount(4);
    auto setRow = [this](int row, const char *leaf, const cpuid_result_t *r) {
        m_rawTable->setItem(row, 0, new QTableWidgetItem(leaf));
        m_rawTable->setItem(row, 1, new QTableWidgetItem(QString("0x%1").arg(r->eax, 8, 16, QChar('0'))));
        m_rawTable->setItem(row, 2, new QTableWidgetItem(QString("0x%1").arg(r->ebx, 8, 16, QChar('0'))));
        m_rawTable->setItem(row, 3, new QTableWidgetItem(QString("0x%1").arg(r->ecx, 8, 16, QChar('0'))));
        m_rawTable->setItem(row, 4, new QTableWidgetItem(QString("0x%1").arg(r->edx, 8, 16, QChar('0'))));
    };

    setRow(0, "Leaf 0", &cpu->leaf0);
    setRow(1, "Leaf 1", &cpu->leaf1);
    setRow(2, "Leaf 7", &cpu->leaf7);
    setRow(3, "0x80000001", &cpu->leaf80000001);
}