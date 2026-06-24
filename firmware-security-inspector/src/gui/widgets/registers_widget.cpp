#include "registers_widget.h"
#include <QVBoxLayout>
#include <QHeaderView>
#include <QFont>

RegistersWidget::RegistersWidget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 12, 12, 12);

    m_table = new QTableWidget(0, 2);
    m_table->setHorizontalHeaderLabels({"Register", "Value"});
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_table->verticalHeader()->hide();
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setAlternatingRowColors(true);
    m_table->setShowGrid(false);
    m_table->setFont(QFont("Consolas", 9));

    layout->addWidget(m_table);
}

void RegistersWidget::updateData(const fsi_registers_t *regs)
{
    if (!regs) return;

    m_table->setRowCount(0);

    auto addRow = [this](const QString &field, const QString &value) {
        int row = m_table->rowCount();
        m_table->insertRow(row);
        m_table->setItem(row, 0, new QTableWidgetItem(field));
        m_table->setItem(row, 1, new QTableWidgetItem(value));
    };

    if (!regs->available) {
        addRow("Status", "Not Available");
        addRow("Error", QString::fromLocal8Bit(regs->error_msg));
        return;
    }

    addRow("CR0", QString("0x%1").arg(regs->cr0, 16, 16, QChar('0')));
    addRow("CR2", QString("0x%1").arg(regs->cr2, 16, 16, QChar('0')));
    addRow("CR3", QString("0x%1").arg(regs->cr3, 16, 16, QChar('0')));
    addRow("CR4", QString("0x%1").arg(regs->cr4, 16, 16, QChar('0')));
    addRow("RFLAGS", QString("0x%1").arg(regs->rflags, 16, 16, QChar('0')));


    if (regs->cr4) {
        QString bits;
        if (regs->cr4 & CR4_SMEP) bits += "SMEP ";
        if (regs->cr4 & CR4_SMAP) bits += "SMAP ";
        if (regs->cr4 & CR4_UMIP) bits += "UMIP ";
        if (regs->cr4 & CR4_CET) bits += "CET ";
        if (regs->cr4 & CR4_VMXE) bits += "VMXE ";
        if (regs->cr4 & CR4_PAE) bits += "PAE ";
        if (!bits.isEmpty()) {
            addRow("CR4 Features", bits.trimmed());
        }
    }
}