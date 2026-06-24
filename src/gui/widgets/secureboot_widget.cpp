#include "secureboot_widget.h"
#include <QVBoxLayout>
#include <QHeaderView>
#include <QFont>

SecureBootWidget::SecureBootWidget(QWidget *parent)
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

void SecureBootWidget::updateData(const fsi_secureboot_info_t *sb)
{
    if (!sb) return;

    m_table->setRowCount(0);

    auto addRow = [this](const QString &field, const QString &value) {
        int row = m_table->rowCount();
        m_table->insertRow(row);
        m_table->setItem(row, 0, new QTableWidgetItem(field));
        m_table->setItem(row, 1, new QTableWidgetItem(value));
    };

    addRow("State", QString::fromLocal8Bit(sb->state_str));
    addRow("PK Present", sb->pk_present ? "YES" : "NO");
    addRow("KEK Present", sb->kek_present ? "YES" : "NO");
    addRow("DB Present", sb->db_present ? "YES" : "NO");
    addRow("DBX Present", sb->dbx_present ? "YES" : "NO");
    addRow("MOK Present", sb->mok_present ? "YES" : "NO");

    if (sb->error_msg[0] != '\0') {
        addRow("Error", QString::fromLocal8Bit(sb->error_msg));
    }
}