#include "smbios_widget.h"
#include <QVBoxLayout>
#include <QHeaderView>
#include <QFont>

SmbiosWidget::SmbiosWidget(QWidget *parent)
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

void SmbiosWidget::updateData(const fsi_smbios_info_t *smbios)
{
    if (!smbios) return;

    m_table->setRowCount(0);

    auto addRow = [this](const QString &field, const QString &value) {
        int row = m_table->rowCount();
        m_table->insertRow(row);
        m_table->setItem(row, 0, new QTableWidgetItem(field));
        m_table->setItem(row, 1, new QTableWidgetItem(value));
    };

    if (!smbios->available) {
        addRow("Status", "Not Available");
        addRow("Error", QString::fromLocal8Bit(smbios->error_msg));
        return;
    }

    if (smbios->smbios_major > 0) {
        addRow("SMBIOS Version", QString("%1.%2").arg(smbios->smbios_major).arg(smbios->smbios_minor));
    }
    addRow("BIOS Vendor", QString::fromLocal8Bit(smbios->bios_vendor));
    addRow("BIOS Version", QString::fromLocal8Bit(smbios->bios_version));
    addRow("BIOS Date", QString::fromLocal8Bit(smbios->bios_release_date));
    addRow("System Manufacturer", QString::fromLocal8Bit(smbios->system_manufacturer));
    addRow("System Product", QString::fromLocal8Bit(smbios->system_product));
    addRow("System Version", QString::fromLocal8Bit(smbios->system_version));
    addRow("System Serial", QString::fromLocal8Bit(smbios->system_serial));
    addRow("System UUID", QString::fromLocal8Bit(smbios->system_uuid));
    addRow("Board Manufacturer", QString::fromLocal8Bit(smbios->baseboard_manufacturer));
    addRow("Board Product", QString::fromLocal8Bit(smbios->baseboard_product));
    addRow("Board Serial", QString::fromLocal8Bit(smbios->baseboard_serial));
}