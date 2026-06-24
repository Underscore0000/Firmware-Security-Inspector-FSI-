#include "firmware_widget.h"
#include <QVBoxLayout>
#include <QHeaderView>
#include <QFont>

FirmwareWidget::FirmwareWidget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 12, 12, 12);

    m_table = new QTableWidget(0, 2);
    m_table->setHorizontalHeaderLabels({"Field", "Value"});
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_table->verticalHeader()->hide();
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setAlternatingRowColors(true);
    m_table->setShowGrid(false);
    m_table->setFont(QFont("Consolas", 9));

    layout->addWidget(m_table);
}

void FirmwareWidget::updateData(const fsi_firmware_info_t *firmware)
{
    if (!firmware) return;

    m_table->setRowCount(0);

    auto addRow = [this](const QString &field, const QString &value) {
        int row = m_table->rowCount();
        m_table->insertRow(row);
        m_table->setItem(row, 0, new QTableWidgetItem(field));
        m_table->setItem(row, 1, new QTableWidgetItem(value));
    };

    addRow("Boot Mode", QString::fromLocal8Bit(firmware->boot_mode_str));
    addRow("BIOS Vendor", QString::fromLocal8Bit(firmware->bios_vendor));
    addRow("BIOS Version", QString::fromLocal8Bit(firmware->bios_version));
    addRow("BIOS Date", QString::fromLocal8Bit(firmware->bios_date));
    addRow("Manufacturer", QString::fromLocal8Bit(firmware->system_manufacturer));
    addRow("Product", QString::fromLocal8Bit(firmware->system_product));
    addRow("Version", QString::fromLocal8Bit(firmware->system_version));

    if (firmware->boot_mode == 1) {
        addRow("UEFI Vendor", QString::fromLocal8Bit(firmware->uefi_vendor));
        addRow("UEFI Version", QString::fromLocal8Bit(firmware->uefi_version));
        addRow("UEFI Date", QString::fromLocal8Bit(firmware->uefi_release_date));
    }
}