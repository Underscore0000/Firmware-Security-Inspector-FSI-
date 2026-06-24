#include "developer_widget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QFont>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>

DeveloperWidget::DeveloperWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void DeveloperWidget::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(12, 12, 12, 12);
    mainLayout->setSpacing(8);

    // Header
    QLabel *title = new QLabel("Developer Mode");
    title->setStyleSheet(
        "color:#569cd6;font-size:16px;font-weight:bold;"
        "border-bottom:1px solid #3f3f46;padding-bottom:4px;");
    mainLayout->addWidget(title);

    // Status
    m_statusLabel = new QLabel("Ready");
    m_statusLabel->setStyleSheet("color:#858585;font-size:11px;");
    mainLayout->addWidget(m_statusLabel);

    // Tab widget
    m_tabWidget = new QTabWidget;
    m_tabWidget->setStyleSheet(
        "QTabWidget::pane{background:#1e1e1e;border:1px solid #3f3f46;}"
        "QTabBar::tab{background:#2d2d30;color:#d4d4d4;padding:6px 14px;"
        "border:1px solid #3f3f46;border-bottom:none;}"
        "QTabBar::tab:selected{background:#094771;color:#ffffff;}");

    // CPU Raw tab
    m_cpuRawTable = new QTableWidget(0, 5);
    m_cpuRawTable->setHorizontalHeaderLabels({"Leaf", "EAX", "EBX", "ECX", "EDX"});
    m_cpuRawTable->horizontalHeader()->setStretchLastSection(true);
    m_cpuRawTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_cpuRawTable->verticalHeader()->hide();
    m_cpuRawTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_cpuRawTable->setAlternatingRowColors(true);
    m_cpuRawTable->setShowGrid(false);
    m_cpuRawTable->setFont(QFont("Consolas", 9));
    m_tabWidget->addTab(m_cpuRawTable, "CPU Raw Data");

    // Registers tab
    m_registerTable = new QTableWidget(0, 3);
    m_registerTable->setHorizontalHeaderLabels({"Register", "Value", "Description"});
    m_registerTable->horizontalHeader()->setStretchLastSection(true);
    m_registerTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_registerTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_registerTable->verticalHeader()->hide();
    m_registerTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_registerTable->setAlternatingRowColors(true);
    m_registerTable->setShowGrid(false);
    m_registerTable->setFont(QFont("Consolas", 9));
    m_tabWidget->addTab(m_registerTable, "Registers");

    // Hex dump tab
    m_hexEdit = new QTextEdit;
    m_hexEdit->setReadOnly(true);
    m_hexEdit->setFont(QFont("Consolas", 9));
    m_hexEdit->setStyleSheet(
        "QTextEdit{background:#1a1a1a;color:#d4d4d4;border:none;"
        "font-family:Consolas;font-size:9px;}");
    m_tabWidget->addTab(m_hexEdit, "System Hex Dump");

    mainLayout->addWidget(m_tabWidget);
}

void DeveloperWidget::updateData(const fsi_system_snapshot_t *snap)
{
    if (!snap) return;

    populateCpuRawTab(&snap->cpu);
    populateRegisterTab(&snap->registers);
    populateHexTab(snap);

    m_statusLabel->setText("Data loaded");
}

void DeveloperWidget::populateCpuRawTab(const fsi_cpu_info_t *cpu)
{
    m_cpuRawTable->setRowCount(0);

    struct RawLeaf {
        const char *name;
        uint32_t eax, ebx, ecx, edx;
    };

    // Collect all CPUID leaves
    QList<RawLeaf> leaves;
    leaves.append({"Leaf 0", cpu->leaf0.eax, cpu->leaf0.ebx, cpu->leaf0.ecx, cpu->leaf0.edx});
    leaves.append({"Leaf 1", cpu->leaf1.eax, cpu->leaf1.ebx, cpu->leaf1.ecx, cpu->leaf1.edx});
    leaves.append({"Leaf 7", cpu->leaf7.eax, cpu->leaf7.ebx, cpu->leaf7.ecx, cpu->leaf7.edx});
    leaves.append({"0x80000001", cpu->leaf80000001.eax, cpu->leaf80000001.ebx,
                   cpu->leaf80000001.ecx, cpu->leaf80000001.edx});

    // Add extended leaves if available
    uint32_t max_ext = fsi_cpuid_max_extended_leaf();
    for (uint32_t leaf = 0x80000002; leaf <= 0x80000004 && leaf <= max_ext; leaf++) {
        cpuid_result_t r;
        fsi_cpuid(leaf, 0, &r);
        leaves.append({QString("0x%1").arg(leaf, 8, 16, QChar('0')).toLocal8Bit().constData(),
                       r.eax, r.ebx, r.ecx, r.edx});
    }

    for (const RawLeaf &l : leaves) {
        int row = m_cpuRawTable->rowCount();
        m_cpuRawTable->insertRow(row);
        m_cpuRawTable->setItem(row, 0, new QTableWidgetItem(l.name));
        m_cpuRawTable->setItem(row, 1, new QTableWidgetItem(QString("0x%1").arg(l.eax, 8, 16, QChar('0'))));
        m_cpuRawTable->setItem(row, 2, new QTableWidgetItem(QString("0x%1").arg(l.ebx, 8, 16, QChar('0'))));
        m_cpuRawTable->setItem(row, 3, new QTableWidgetItem(QString("0x%1").arg(l.ecx, 8, 16, QChar('0'))));
        m_cpuRawTable->setItem(row, 4, new QTableWidgetItem(QString("0x%1").arg(l.edx, 8, 16, QChar('0'))));
    }
}

void DeveloperWidget::populateRegisterTab(const fsi_registers_t *regs)
{
    m_registerTable->setRowCount(0);

    auto addRow = [this](const QString &name, const QString &value, const QString &desc) {
        int row = m_registerTable->rowCount();
        m_registerTable->insertRow(row);
        m_registerTable->setItem(row, 0, new QTableWidgetItem(name));
        m_registerTable->setItem(row, 1, new QTableWidgetItem(value));
        m_registerTable->setItem(row, 2, new QTableWidgetItem(desc));
    };

    if (!regs->available) {
        addRow("Status", "N/A", QString::fromLocal8Bit(regs->error_msg));
        return;
    }

    addRow("CR0", QString("0x%1").arg(regs->cr0, 16, 16, QChar('0')),
           "Control Register 0 - Protection Enable, Paging, WP, etc.");
    addRow("CR2", QString("0x%1").arg(regs->cr2, 16, 16, QChar('0')),
           "Control Register 2 - Page Fault Linear Address");
    addRow("CR3", QString("0x%1").arg(regs->cr3, 16, 16, QChar('0')),
           "Control Register 3 - Page Directory Base Register");
    addRow("CR4", QString("0x%1").arg(regs->cr4, 16, 16, QChar('0')),
           "Control Register 4 - SMEP, SMAP, UMIP, CET, etc.");
    addRow("RFLAGS", QString("0x%1").arg(regs->rflags, 16, 16, QChar('0')),
           "RFLAGS Register - Status flags");

    // CR4 bit decode
    if (regs->cr4) {
        QString bits;
        if (regs->cr4 & CR4_SMEP) bits += "SMEP ";
        if (regs->cr4 & CR4_SMAP) bits += "SMAP ";
        if (regs->cr4 & CR4_UMIP) bits += "UMIP ";
        if (regs->cr4 & CR4_CET) bits += "CET ";
        if (regs->cr4 & CR4_VMXE) bits += "VMXE ";
        if (regs->cr4 & CR4_PAE) bits += "PAE ";
        if (!bits.isEmpty()) {
            addRow("CR4 Bits", bits.trimmed(), "Enabled security features in CR4");
        }
    }
}

void DeveloperWidget::populateHexTab(const fsi_system_snapshot_t *snap)
{
    // Generate a hex dump of key system structures
    QString dump;

    // CPU vendor and brand
    dump += "=== CPU Information ===\n";
    dump += QString("Vendor: %1\n").arg(QString::fromLocal8Bit(snap->cpu.vendor));
    dump += QString("Brand: %1\n").arg(QString::fromLocal8Bit(snap->cpu.brand));
    dump += QString("Microarchitecture: %1\n\n").arg(QString::fromLocal8Bit(snap->cpu.microarch));

    // CPU features
    dump += "=== CPU Security Features ===\n";
#define DUMP_FEAT(name, val) dump += QString("%1: %2\n").arg(name, 14).arg((val) ? "YES" : "NO")
    DUMP_FEAT("NX/XD", snap->cpu.features.nx);
    DUMP_FEAT("SMEP", snap->cpu.features.smep);
    DUMP_FEAT("SMAP", snap->cpu.features.smap);
    DUMP_FEAT("UMIP", snap->cpu.features.umip);
    DUMP_FEAT("CET-SS", snap->cpu.features.cet_ss);
    DUMP_FEAT("CET-IBT", snap->cpu.features.cet_ibt);
    DUMP_FEAT("AES-NI", snap->cpu.features.aes_ni);
    DUMP_FEAT("VT-x", snap->cpu.features.vtx);
    DUMP_FEAT("AMD-V", snap->cpu.features.amd_v);
    DUMP_FEAT("SGX", snap->cpu.features.sgx);
    DUMP_FEAT("TXT", snap->cpu.features.txt);
    DUMP_FEAT("AVX2", snap->cpu.features.avx2);
    DUMP_FEAT("AVX-512", snap->cpu.features.avx512f);
    dump += "\n";

    // Secure Boot
    dump += "=== Secure Boot ===\n";
    dump += QString("State: %1\n").arg(QString::fromLocal8Bit(snap->secureboot.state_str));
    dump += QString("PK: %1, KEK: %2, DB: %3, DBX: %4\n\n")
        .arg(snap->secureboot.pk_present ? "YES" : "NO")
        .arg(snap->secureboot.kek_present ? "YES" : "NO")
        .arg(snap->secureboot.db_present ? "YES" : "NO")
        .arg(snap->secureboot.dbx_present ? "YES" : "NO");

    // TPM
    dump += "=== TPM ===\n";
    dump += QString("Present: %1, Version: %2, Enabled: %3\n")
        .arg(snap->tpm.present ? "YES" : "NO")
        .arg(QString::fromLocal8Bit(snap->tpm.version_str))
        .arg(snap->tpm.enabled ? "YES" : "NO");
    dump += QString("Manufacturer: %1\n\n").arg(QString::fromLocal8Bit(snap->tpm.manufacturer));

    // ACPI Tables
    dump += "=== ACPI Tables ===\n";
    for (int i = 0; i < snap->acpi.count && i < 20; i++) {
        const fsi_acpi_table_entry_t *t = &snap->acpi.tables[i];
        dump += QString("%1: length=%2, rev=%3, checksum=%4\n")
            .arg(QString::fromLocal8Bit(t->signature))
            .arg(t->length)
            .arg(t->revision)
            .arg(t->checksum_valid ? "OK" : "FAIL");
    }
    if (snap->acpi.count > 20) {
        dump += QString("... and %1 more tables\n").arg(snap->acpi.count - 20);
    }
    dump += "\n";

    // MSRs
    dump += "=== MSR Values (security-relevant) ===\n";
    for (int i = 0; i < snap->msr.count; i++) {
        const fsi_msr_entry_t *e = &snap->msr.entries[i];
        if (e->readable) {
            dump += QString("0x%1: %2 = 0x%3\n")
                .arg(e->index, 8, 16, QChar('0'))
                .arg(QString::fromLocal8Bit(e->name), 24)
                .arg(e->value, 16, 16, QChar('0'));
        }
    }
    dump += "\n";

    // Audit score
    dump += "=== Security Audit ===\n";
    dump += QString("Score: %1/100 [Grade: %2]\n")
        .arg(snap->audit.score)
        .arg(QString::fromLocal8Bit(snap->audit.grade));

    m_hexEdit->setPlainText(dump);
}