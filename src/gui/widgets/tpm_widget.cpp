#include "tpm_widget.h"
#include <QVBoxLayout>
#include <QHeaderView>
#include <QFont>

TpmWidget::TpmWidget(QWidget *parent)
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

void TpmWidget::updateData(const fsi_tpm_info_t *tpm)
{
    if (!tpm) return;

    m_table->setRowCount(0);

    auto addRow = [this](const QString &field, const QString &value) {
        int row = m_table->rowCount();
        m_table->insertRow(row);
        m_table->setItem(row, 0, new QTableWidgetItem(field));
        m_table->setItem(row, 1, new QTableWidgetItem(value));
    };

    addRow("Present", tpm->present ? "YES" : "NO");
    
    if (!tpm->present) {
        addRow("Error", QString::fromLocal8Bit(tpm->error_msg));
        return;
    }

    addRow("Version", QString::fromLocal8Bit(tpm->version_str));
    addRow("Manufacturer", QString::fromLocal8Bit(tpm->manufacturer));
    addRow("Enabled", tpm->enabled ? "YES" : "NO");
    addRow("Activated", tpm->activated ? "YES" : "NO");
    addRow("Owned", tpm->owned ? "YES" : "NO");
}