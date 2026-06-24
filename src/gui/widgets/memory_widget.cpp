#include "memory_widget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QFont>
#include <QLabel>

MemoryWidget::MemoryWidget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 12, 12, 12);

    m_table = new QTableWidget(0, 4);
    m_table->setHorizontalHeaderLabels({"Base", "Length", "Type", "Description"});
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_table->verticalHeader()->hide();
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setAlternatingRowColors(true);
    m_table->setShowGrid(false);
    m_table->setFont(QFont("Consolas", 9));

    layout->addWidget(m_table);
}

void MemoryWidget::updateData(const fsi_memory_map_t *memory)
{
    if (!memory) return;

    m_table->setRowCount(0);

    if (!memory->available) {
        m_table->setRowCount(1);
        m_table->setItem(0, 0, new QTableWidgetItem("Not Available"));
        m_table->setItem(0, 3, new QTableWidgetItem(QString::fromLocal8Bit(memory->error_msg)));
        return;
    }

    for (int i = 0; i < memory->count; i++) {
        const fsi_mem_range_t *r = &memory->ranges[i];
        int row = m_table->rowCount();
        m_table->insertRow(row);

        m_table->setItem(row, 0, new QTableWidgetItem(QString("0x%1").arg(r->base, 16, 16, QChar('0'))));
        m_table->setItem(row, 1, new QTableWidgetItem(QString("0x%1").arg(r->length, 16, 16, QChar('0'))));
        
        QString typeStr;
        switch(r->type) {
            case MEM_TYPE_USABLE: typeStr = "USABLE"; break;
            case MEM_TYPE_RESERVED: typeStr = "RESERVED"; break;
            case MEM_TYPE_ACPI_RECLAIMABLE: typeStr = "ACPI"; break;
            case MEM_TYPE_ACPI_NVS: typeStr = "ACPI NVS"; break;
            case MEM_TYPE_BAD: typeStr = "BAD"; break;
            case MEM_TYPE_PERSISTENT: typeStr = "PERSISTENT"; break;
            default: typeStr = "UNKNOWN"; break;
        }
        m_table->setItem(row, 2, new QTableWidgetItem(typeStr));
        m_table->setItem(row, 3, new QTableWidgetItem(QString::fromLocal8Bit(r->type_str)));
    }
}