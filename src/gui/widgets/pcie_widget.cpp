#include "pcie_widget.h"
#include <QVBoxLayout>
#include <QHeaderView>
#include <QFont>

PcieWidget::PcieWidget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 12, 12, 12);

    m_table = new QTableWidget(0, 6);
    m_table->setHorizontalHeaderLabels({"Bus", "D:F", "Vendor", "Device", "Type", "Class"});
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    m_table->verticalHeader()->hide();
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setAlternatingRowColors(true);
    m_table->setShowGrid(false);
    m_table->setFont(QFont("Consolas", 9));

    layout->addWidget(m_table);
}

void PcieWidget::updateData(const fsi_pcie_data_t *pcie)
{
    if (!pcie) return;

    m_table->setRowCount(0);

    if (!pcie->available) {
        m_table->setRowCount(1);
        m_table->setItem(0, 0, new QTableWidgetItem("Not Available"));
        m_table->setItem(0, 5, new QTableWidgetItem(QString::fromLocal8Bit(pcie->error_msg)));
        return;
    }

    for (int i = 0; i < pcie->count; i++) {
        const fsi_pci_device_t *d = &pcie->devices[i];
        int row = m_table->rowCount();
        m_table->insertRow(row);

        m_table->setItem(row, 0, new QTableWidgetItem(QString("%1").arg(d->bus, 2, 16, QChar('0'))));
        m_table->setItem(row, 1, new QTableWidgetItem(QString("%1:%2").arg(d->device, 2, 16, QChar('0')).arg(d->function)));
        m_table->setItem(row, 2, new QTableWidgetItem(QString("0x%1").arg(d->vendor_id, 4, 16, QChar('0'))));
        m_table->setItem(row, 3, new QTableWidgetItem(QString("0x%1").arg(d->device_id, 4, 16, QChar('0'))));
        m_table->setItem(row, 4, new QTableWidgetItem(d->is_pcie ? "PCIe" : "PCI"));
        m_table->setItem(row, 5, new QTableWidgetItem(QString::fromLocal8Bit(d->class_name)));
    }
}