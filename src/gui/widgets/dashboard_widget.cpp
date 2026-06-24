#include "dashboard_widget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QFont>
#include <QGroupBox>
#include <QLabel>
#include <QTableWidget>

DashboardWidget::DashboardWidget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(12, 12, 12, 12);
    mainLayout->setSpacing(8);

    QLabel *title = new QLabel("Dashboard");
    title->setStyleSheet(
        "color:#569cd6;font-size:16px;font-weight:bold;"
        "border-bottom:1px solid #3f3f46;padding-bottom:4px;");
    mainLayout->addWidget(title);

    /* Score area */
    QHBoxLayout *scoreLayout = new QHBoxLayout;

    QGroupBox *scoreBox = new QGroupBox("Security Score");
    scoreBox->setFixedWidth(220);
    scoreBox->setStyleSheet(
        "QGroupBox{border:1px solid #3f3f46;color:#9cdcfe;"
        "font-weight:bold;padding-top:8px;}"
        "QGroupBox::title{subcontrol-position:top left;padding:0 4px;}");
    QVBoxLayout *sbLayout = new QVBoxLayout(scoreBox);

    QHBoxLayout *scoreRow = new QHBoxLayout;
    m_scoreLabel = new QLabel("--");
    m_scoreLabel->setStyleSheet(
        "color:#4fc1ff;font-size:52px;font-weight:bold;font-family:Consolas;");
    m_gradeLabel = new QLabel("[?]");
    m_gradeLabel->setStyleSheet(
        "color:#9cdcfe;font-size:24px;font-family:Consolas;");
    scoreRow->addWidget(m_scoreLabel);
    scoreRow->addWidget(m_gradeLabel);
    scoreRow->addStretch();
    sbLayout->addLayout(scoreRow);
    QLabel *scoreMax = new QLabel("/ 100");
    scoreMax->setStyleSheet("color:#858585;font-size:11px;");
    sbLayout->addWidget(scoreMax);

    scoreLayout->addWidget(scoreBox);

    QGroupBox *checksBox = new QGroupBox("Security Checks Summary");
    checksBox->setStyleSheet(scoreBox->styleSheet());
    QVBoxLayout *cbLayout = new QVBoxLayout(checksBox);

    m_checksTable = new QTableWidget(0, 3);
    m_checksTable->setHorizontalHeaderLabels({"Check", "Result", "Score"});
    m_checksTable->horizontalHeader()->setStretchLastSection(false);
    m_checksTable->horizontalHeader()->setSectionResizeMode(
        0, QHeaderView::Stretch);
    m_checksTable->horizontalHeader()->setSectionResizeMode(
        1, QHeaderView::ResizeToContents);
    m_checksTable->horizontalHeader()->setSectionResizeMode(
        2, QHeaderView::ResizeToContents);
    m_checksTable->verticalHeader()->hide();
    m_checksTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_checksTable->setAlternatingRowColors(true);
    m_checksTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_checksTable->setShowGrid(false);
    m_checksTable->setFont(QFont("Consolas", 9));
    cbLayout->addWidget(m_checksTable);
    scoreLayout->addWidget(checksBox, 1);

    mainLayout->addLayout(scoreLayout);

    /* System info */
    QHBoxLayout *infoLayout = new QHBoxLayout;

    QGroupBox *sysBox = new QGroupBox("System");
    sysBox->setStyleSheet(scoreBox->styleSheet());
    QVBoxLayout *sysLayout = new QVBoxLayout(sysBox);
    m_systemTable = new QTableWidget(0, 2);
    m_systemTable->setHorizontalHeaderLabels({"Field", "Value"});
    m_systemTable->horizontalHeader()->setStretchLastSection(true);
    m_systemTable->horizontalHeader()->setSectionResizeMode(
        0, QHeaderView::ResizeToContents);
    m_systemTable->verticalHeader()->hide();
    m_systemTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_systemTable->setAlternatingRowColors(true);
    m_systemTable->setShowGrid(false);
    m_systemTable->setFont(QFont("Consolas", 9));
    sysLayout->addWidget(m_systemTable);
    infoLayout->addWidget(sysBox);

    QGroupBox *cpuBox = new QGroupBox("CPU");
    cpuBox->setStyleSheet(scoreBox->styleSheet());
    QVBoxLayout *cpuLayout = new QVBoxLayout(cpuBox);
    m_cpuTable = new QTableWidget(0, 2);
    m_cpuTable->setHorizontalHeaderLabels({"Field", "Value"});
    m_cpuTable->horizontalHeader()->setStretchLastSection(true);
    m_cpuTable->horizontalHeader()->setSectionResizeMode(
        0, QHeaderView::ResizeToContents);
    m_cpuTable->verticalHeader()->hide();
    m_cpuTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_cpuTable->setAlternatingRowColors(true);
    m_cpuTable->setShowGrid(false);
    m_cpuTable->setFont(QFont("Consolas", 9));
    cpuLayout->addWidget(m_cpuTable);
    infoLayout->addWidget(cpuBox);

    mainLayout->addLayout(infoLayout, 1);
}

static QColor resultColor(fsi_audit_result_t r)
{
    switch (r) {
        case AUDIT_PASS:    return QColor(78, 201, 176);
        case AUDIT_WARNING: return QColor(220, 220, 170);
        case AUDIT_FAIL:    return QColor(244, 71, 71);
        default:            return QColor(156, 220, 254);
    }
}

void DashboardWidget::updateData(const fsi_system_snapshot_t *snap)
{
    if (!snap) return;

    /* Score */
    m_scoreLabel->setText(QString::number(snap->audit.score));
    m_gradeLabel->setText(
        QString("[%1]").arg(QString::fromLocal8Bit(snap->audit.grade)));

    /* Color grade */
    QColor gradeColor = (snap->audit.score >= 75) ? QColor(78, 201, 176)
                      : (snap->audit.score >= 50) ? QColor(220, 220, 170)
                      : QColor(244, 71, 71);
    m_gradeLabel->setStyleSheet(
        QString("color:%1;font-size:24px;font-family:Consolas;")
        .arg(gradeColor.name()));

    /* Checks table */
    m_checksTable->setRowCount(0);
    for (int i = 0; i < snap->audit.check_count; i++) {
        const fsi_audit_check_t *c = &snap->audit.checks[i];
        int row = m_checksTable->rowCount();
        m_checksTable->insertRow(row);
        m_checksTable->setItem(row, 0,
            new QTableWidgetItem(
                QString::fromLocal8Bit(c->name)));

        QTableWidgetItem *resItem = new QTableWidgetItem(
            QString::fromLocal8Bit(fsi_audit_result_str(c->result)));
        resItem->setForeground(resultColor(c->result));
        resItem->setFont(QFont("Consolas", 9, QFont::Bold));
        m_checksTable->setItem(row, 1, resItem);

        m_checksTable->setItem(row, 2,
            new QTableWidgetItem(
                QString("%1/%2")
                .arg(c->score_contribution)
                .arg(c->max_contribution)));
    }

    /* System table */
    m_systemTable->setRowCount(0);
    auto addRow = [](QTableWidget *t, const QString &f, const QString &v) {
        int r = t->rowCount(); t->insertRow(r);
        t->setItem(r, 0, new QTableWidgetItem(f));
        t->setItem(r, 1, new QTableWidgetItem(v));
    };
    addRow(m_systemTable, "Manufacturer",
           QString::fromLocal8Bit(snap->firmware.system_manufacturer));
    addRow(m_systemTable, "Product",
           QString::fromLocal8Bit(snap->firmware.system_product));
    addRow(m_systemTable, "Boot Mode",
           QString::fromLocal8Bit(snap->firmware.boot_mode_str));
    addRow(m_systemTable, "Secure Boot",
           QString::fromLocal8Bit(snap->secureboot.state_str));
    addRow(m_systemTable, "TPM",
           snap->tpm.present
           ? QString("Present (%1)").arg(
               QString::fromLocal8Bit(snap->tpm.version_str))
           : "Not Present");
    addRow(m_systemTable, "BIOS Version",
           QString::fromLocal8Bit(snap->firmware.bios_version));

    /* CPU table */
    m_cpuTable->setRowCount(0);
    addRow(m_cpuTable, "Vendor",
           QString::fromLocal8Bit(snap->cpu.vendor));
    addRow(m_cpuTable, "Brand",
           QString::fromLocal8Bit(snap->cpu.brand));
    addRow(m_cpuTable, "Microarchitecture",
           QString::fromLocal8Bit(snap->cpu.microarch));
    addRow(m_cpuTable, "Cores",
           QString("%1 physical, %2 logical")
           .arg(snap->cpu.physical_cores)
           .arg(snap->cpu.logical_cores));
    addRow(m_cpuTable, "SMEP",
           snap->cpu.features.smep ? "YES" : "NO");
    addRow(m_cpuTable, "SMAP",
           snap->cpu.features.smap ? "YES" : "NO");
}