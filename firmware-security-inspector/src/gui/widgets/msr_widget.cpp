#include "msr_widget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QFont>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QMessageBox>

MsrWidget::MsrWidget(QWidget *parent)
    : QWidget(parent)
    , m_currentData(nullptr)
{
    setupUI();
}

void MsrWidget::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(12, 12, 12, 12);
    mainLayout->setSpacing(8);

    // Header
    QLabel *title = new QLabel("Model Specific Registers (MSR)");
    title->setStyleSheet(
        "color:#569cd6;font-size:16px;font-weight:bold;"
        "border-bottom:1px solid #3f3f46;padding-bottom:4px;");
    mainLayout->addWidget(title);

    // Toolbar
    QHBoxLayout *toolbar = new QHBoxLayout;
    m_filterEdit = new QLineEdit;
    m_filterEdit->setPlaceholderText("Filter MSR by name or index...");
    m_filterEdit->setStyleSheet(
        "QLineEdit{background:#252526;color:#d4d4d4;border:1px solid #3f3f46;"
        "padding:4px 8px;border-radius:2px;}");
    m_filterEdit->setFixedWidth(300);
    connect(m_filterEdit, &QLineEdit::textChanged, this, &MsrWidget::onFilterChanged);

    m_refreshBtn = new QPushButton("⟳ Refresh");
    m_refreshBtn->setStyleSheet(
        "QPushButton{background:#37373d;color:#d4d4d4;"
        "border:1px solid #555;padding:4px 12px;border-radius:2px;}"
        "QPushButton:hover{background:#4a4a52;}");
    connect(m_refreshBtn, &QPushButton::clicked, this, &MsrWidget::onRefreshClicked);

    m_statusLabel = new QLabel("Ready");
    m_statusLabel->setStyleSheet("color:#858585;font-size:11px;");

    toolbar->addWidget(m_filterEdit);
    toolbar->addWidget(m_refreshBtn);
    toolbar->addStretch();
    toolbar->addWidget(m_statusLabel);
    mainLayout->addLayout(toolbar);

    // Table
    m_table = new QTableWidget(0, 4);
    m_table->setHorizontalHeaderLabels({"Index", "Name", "Value", "Description"});
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_table->verticalHeader()->hide();
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setAlternatingRowColors(true);
    m_table->setShowGrid(false);
    m_table->setFont(QFont("Consolas", 9));
    m_table->setSortingEnabled(true);
    m_table->sortByColumn(0, Qt::AscendingOrder);

    // Style for hex values
    m_table->setStyleSheet(
        "QTableWidget::item{color:#d4d4d4;}"
        "QTableWidget::item:selected{background:#094771;}");

    mainLayout->addWidget(m_table);
}

void MsrWidget::updateData(const fsi_msr_data_t *msr)
{
    m_currentData = msr;
    populateTable(msr);

    if (msr->available) {
        m_statusLabel->setText(QString("Loaded %1 MSR entries").arg(msr->count));
        m_statusLabel->setStyleSheet("color:#4ec9b0;font-size:11px;");
    } else {
        m_statusLabel->setText(QString::fromLocal8Bit(msr->error_msg));
        m_statusLabel->setStyleSheet("color:#f44747;font-size:11px;");
    }
}

void MsrWidget::populateTable(const fsi_msr_data_t *msr)
{
    m_table->setRowCount(0);
    if (!msr) return;

    for (int i = 0; i < msr->count; i++) {
        const fsi_msr_entry_t *e = &msr->entries[i];
        int row = m_table->rowCount();
        m_table->insertRow(row);

        // Index
        QTableWidgetItem *idxItem = new QTableWidgetItem(
            QString("0x%1").arg(e->index, 8, 16, QChar('0')));
        idxItem->setFont(QFont("Consolas", 9));
        m_table->setItem(row, 0, idxItem);

        // Name
        QTableWidgetItem *nameItem = new QTableWidgetItem(
            QString::fromLocal8Bit(e->name));
        nameItem->setFont(QFont("Consolas", 9, QFont::Bold));
        m_table->setItem(row, 1, nameItem);

        // Value
        QString valStr;
        if (e->readable) {
            valStr = QString("0x%1").arg(e->value, 16, 16, QChar('0'));
        } else {
            valStr = "N/A";
        }
        QTableWidgetItem *valItem = new QTableWidgetItem(valStr);
        if (!e->readable) {
            valItem->setForeground(QColor(100, 100, 100));
        } else {
            valItem->setForeground(QColor(220, 220, 170));
        }
        valItem->setFont(QFont("Consolas", 9));
        m_table->setItem(row, 2, valItem);

        // Description
        QString desc = QString::fromLocal8Bit(e->description);
        if (desc.length() > 80) {
            desc = desc.left(77) + "...";
        }
        QTableWidgetItem *descItem = new QTableWidgetItem(desc);
        descItem->setToolTip(QString::fromLocal8Bit(e->description));
        m_table->setItem(row, 3, descItem);
    }
}

void MsrWidget::onFilterChanged(const QString &text)
{
    if (!m_currentData) return;

    QString filter = text.toLower().trimmed();
    bool isHex = filter.startsWith("0x");

    m_table->setRowCount(0);
    for (int i = 0; i < m_currentData->count; i++) {
        const fsi_msr_entry_t *e = &m_currentData->entries[i];

        bool match = false;
        if (filter.isEmpty()) {
            match = true;
        } else if (isHex) {
            // Filter by index
            bool ok;
            uint32_t idx = filter.toUInt(&ok, 0);
            if (ok && e->index == idx) match = true;
        } else {
            // Filter by name or description
            QString name = QString::fromLocal8Bit(e->name).toLower();
            QString desc = QString::fromLocal8Bit(e->description).toLower();
            if (name.contains(filter) || desc.contains(filter)) match = true;
        }

        if (match) {
            int row = m_table->rowCount();
            m_table->insertRow(row);
            
        }
    }


    populateTable(m_currentData);
    for (int row = m_table->rowCount() - 1; row >= 0; row--) {
        QString name = m_table->item(row, 1)->text().toLower();
        QString desc = m_table->item(row, 3)->text().toLower();
        QString idx = m_table->item(row, 0)->text().toLower();

        bool match = filter.isEmpty();
        if (!match) {
            bool isHex = filter.startsWith("0x");
            if (isHex) {
                match = idx.contains(filter);
            } else {
                match = name.contains(filter) || desc.contains(filter);
            }
        }
        if (!match) {
            m_table->removeRow(row);
        }
    }

    m_statusLabel->setText(QString("Filter: %1 matches").arg(m_table->rowCount()));
}

void MsrWidget::onRefreshClicked()
{
    
    QMessageBox::information(this, "Refresh", "Refresh handled by main window.");
}