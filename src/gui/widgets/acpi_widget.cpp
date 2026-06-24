#include "acpi_widget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QFont>
#include <QLabel>
#include <QPushButton>

AcpiWidget::AcpiWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void AcpiWidget::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(12, 12, 12, 12);
    mainLayout->setSpacing(8);

    // Header
    QLabel *title = new QLabel("ACPI Tables");
    title->setStyleSheet(
        "color:#569cd6;font-size:16px;font-weight:bold;"
        "border-bottom:1px solid #3f3f46;padding-bottom:4px;");
    mainLayout->addWidget(title);

    // Summary row
    QHBoxLayout *summaryLayout = new QHBoxLayout;
    m_oemLabel = new QLabel("OEM: --");
    m_oemLabel->setStyleSheet("color:#9cdcfe;font-size:13px;");
    m_countLabel = new QLabel("Tables: --");
    m_countLabel->setStyleSheet("color:#d4d4d4;font-size:13px;");
    m_statusLabel = new QLabel("Status: --");
    m_statusLabel->setStyleSheet("color:#858585;font-size:13px;");

    summaryLayout->addWidget(m_oemLabel);
    summaryLayout->addWidget(m_countLabel);
    summaryLayout->addWidget(m_statusLabel);
    summaryLayout->addStretch();
    mainLayout->addLayout(summaryLayout);

    // Tab widget
    m_tabWidget = new QTabWidget;
    m_tabWidget->setStyleSheet(
        "QTabWidget::pane{background:#1e1e1e;border:1px solid #3f3f46;}"
        "QTabBar::tab{background:#2d2d30;color:#d4d4d4;padding:6px 14px;"
        "border:1px solid #3f3f46;border-bottom:none;}"
        "QTabBar::tab:selected{background:#094771;color:#ffffff;}");

    // Tables tab
    m_tablesTable = new QTableWidget(0, 6);
    m_tablesTable->setHorizontalHeaderLabels(
        {"Signature", "Length", "Revision", "OEM ID", "Checksum", "Description"});
    m_tablesTable->horizontalHeader()->setStretchLastSection(true);
    m_tablesTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_tablesTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_tablesTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_tablesTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    m_tablesTable->verticalHeader()->hide();
    m_tablesTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tablesTable->setAlternatingRowColors(true);
    m_tablesTable->setShowGrid(false);
    m_tablesTable->setFont(QFont("Consolas", 9));
    m_tablesTable->setSortingEnabled(true);
    m_tablesTable->sortByColumn(0, Qt::AscendingOrder);

    m_tabWidget->addTab(m_tablesTable, "Tables");

    // Summary tab
    m_summaryTable = new QTableWidget(0, 2);
    m_summaryTable->setHorizontalHeaderLabels({"Field", "Value"});
    m_summaryTable->horizontalHeader()->setStretchLastSection(true);
    m_summaryTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_summaryTable->verticalHeader()->hide();
    m_summaryTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_summaryTable->setAlternatingRowColors(true);
    m_summaryTable->setShowGrid(false);
    m_summaryTable->setFont(QFont("Consolas", 9));

    m_tabWidget->addTab(m_summaryTable, "Summary");

    mainLayout->addWidget(m_tabWidget);
}

void AcpiWidget::updateData(const fsi_acpi_data_t *acpi)
{
    if (!acpi) return;

    if (acpi->available) {
        m_oemLabel->setText(QString("OEM: %1").arg(QString::fromLocal8Bit(acpi->oem_id)));
        m_countLabel->setText(QString("Tables: %1").arg(acpi->count));
        m_statusLabel->setText("Status: OK");
        m_statusLabel->setStyleSheet("color:#4ec9b0;font-size:13px;");
        m_statusLabel->setToolTip("ACPI tables successfully read from /sys/firmware/acpi/tables");
    } else {
        m_statusLabel->setText(QString::fromLocal8Bit(acpi->error_msg));
        m_statusLabel->setStyleSheet("color:#f44747;font-size:13px;");
    }

    populateTablesTab(acpi);
    populateSummaryTab(acpi);
}

void AcpiWidget::populateTablesTab(const fsi_acpi_data_t *acpi)
{
    m_tablesTable->setRowCount(0);
    if (!acpi->available || !acpi->tables) return;

    for (int i = 0; i < acpi->count; i++) {
        const fsi_acpi_table_entry_t *t = &acpi->tables[i];
        int row = m_tablesTable->rowCount();
        m_tablesTable->insertRow(row);

        // Signature
        QTableWidgetItem *sigItem = new QTableWidgetItem(
            QString::fromLocal8Bit(t->signature));
        sigItem->setFont(QFont("Consolas", 9, QFont::Bold));
        m_tablesTable->setItem(row, 0, sigItem);

        // Length
        m_tablesTable->setItem(row, 1, new QTableWidgetItem(QString::number(t->length)));

        // Revision
        m_tablesTable->setItem(row, 2, new QTableWidgetItem(QString::number(t->revision)));

        // OEM ID
        m_tablesTable->setItem(row, 3, new QTableWidgetItem(QString::fromLocal8Bit(t->oem_id)));

        // Checksum
        QString checksumStr = t->checksum_valid ? "OK" : "FAIL";
        QTableWidgetItem *csItem = new QTableWidgetItem(checksumStr);
        csItem->setForeground(t->checksum_valid ? QColor(78, 201, 176) : QColor(244, 71, 71));
        csItem->setFont(QFont("Consolas", 9, QFont::Bold));
        m_tablesTable->setItem(row, 4, csItem);

        // Description
        QString desc = QString::fromLocal8Bit(
            fsi_acpi_table_description(t->signature));
        QTableWidgetItem *descItem = new QTableWidgetItem(desc);
        descItem->setToolTip(desc);
        m_tablesTable->setItem(row, 5, descItem);
    }
}

void AcpiWidget::populateSummaryTab(const fsi_acpi_data_t *acpi)
{
    m_summaryTable->setRowCount(0);
    auto addRow = [this](const QString &field, const QString &value) {
        int row = m_summaryTable->rowCount();
        m_summaryTable->insertRow(row);
        m_summaryTable->setItem(row, 0, new QTableWidgetItem(field));
        m_summaryTable->setItem(row, 1, new QTableWidgetItem(value));
    };

    addRow("ACPI Revision", QString::number(acpi->acpi_revision));
    addRow("OEM ID", QString::fromLocal8Bit(acpi->oem_id));
    addRow("Total Tables", QString::number(acpi->count));


    if (acpi->tables) {
        QMap<QString, int> sigCount;
        for (int i = 0; i < acpi->count; i++) {
            QString sig = QString::fromLocal8Bit(acpi->tables[i].signature);
            sigCount[sig]++;
        }

        QString sigSummary;
        QList<QString> keys = sigCount.keys();
        for (const QString &key : keys) {
            sigSummary += QString("%1: %2  ").arg(key).arg(sigCount[key]);
        }
        addRow("Table Counts", sigSummary);
    }


    bool hasFACP = false, hasDSDT = false, hasDMAR = false, hasTPM2 = false;
    if (acpi->tables) {
        for (int i = 0; i < acpi->count; i++) {
            QString sig = QString::fromLocal8Bit(acpi->tables[i].signature);
            if (sig == "FACP") hasFACP = true;
            if (sig == "DSDT") hasDSDT = true;
            if (sig == "DMAR") hasDMAR = true;
            if (sig == "TPM2") hasTPM2 = true;
        }
    }
    addRow("FACP Present", hasFACP ? "YES" : "NO");
    addRow("DSDT Present", hasDSDT ? "YES" : "NO");
    addRow("DMAR (VT-d) Present", hasDMAR ? "YES" : "NO");
    addRow("TPM2 Present", hasTPM2 ? "YES" : "NO");
}