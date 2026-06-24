#include "security_audit_widget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QFont>
#include <QLabel>
#include <QGroupBox>
#include <QProgressBar>

SecurityAuditWidget::SecurityAuditWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void SecurityAuditWidget::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(12, 12, 12, 12);
    mainLayout->setSpacing(8);

    // Header
    QLabel *title = new QLabel("Security Audit");
    title->setStyleSheet(
        "color:#569cd6;font-size:16px;font-weight:bold;"
        "border-bottom:1px solid #3f3f46;padding-bottom:4px;");
    mainLayout->addWidget(title);

    // Score box
    m_scoreBox = new QGroupBox("Security Score");
    m_scoreBox->setStyleSheet(
        "QGroupBox{border:1px solid #3f3f46;color:#9cdcfe;"
        "font-weight:bold;padding-top:12px;}"
        "QGroupBox::title{subcontrol-position:top left;padding:0 4px;}");
    QHBoxLayout *scoreLayout = new QHBoxLayout(m_scoreBox);

    QVBoxLayout *scoreLeft = new QVBoxLayout;
    m_scoreLabel = new QLabel("--");
    m_scoreLabel->setStyleSheet(
        "color:#4fc1ff;font-size:64px;font-weight:bold;font-family:Consolas;");
    QLabel *maxLabel = new QLabel("/ 100");
    maxLabel->setStyleSheet("color:#858585;font-size:14px;");
    QHBoxLayout *scoreRow = new QHBoxLayout;
    scoreRow->addWidget(m_scoreLabel);
    scoreRow->addWidget(maxLabel);
    scoreRow->addStretch();
    scoreLeft->addLayout(scoreRow);

    QVBoxLayout *scoreRight = new QVBoxLayout;
    m_gradeLabel = new QLabel("[?]");
    m_gradeLabel->setStyleSheet(
        "color:#9cdcfe;font-size:42px;font-weight:bold;font-family:Consolas;");
    scoreRight->addWidget(m_gradeLabel);
    scoreRight->addStretch();

    m_scoreBar = new QProgressBar;
    m_scoreBar->setFixedHeight(12);
    m_scoreBar->setTextVisible(false);
    m_scoreBar->setRange(0, 100);
    m_scoreBar->setStyleSheet(
        "QProgressBar{background:#2d2d30;border:none;border-radius:6px;}"
        "QProgressBar::chunk{background:qlineargradient(x1:0,y1:0,x2:1,y2:0,"
        "stop:0 #f44747,stop:0.5 #dcdcaa,stop:1 #4ec9b0);border-radius:6px;}");

    QHBoxLayout *barRow = new QHBoxLayout;
    barRow->addWidget(m_scoreBar, 1);
    barRow->addStretch();

    scoreLayout->addLayout(scoreLeft, 1);
    scoreLayout->addLayout(scoreRight);
    mainLayout->addWidget(m_scoreBox);


    QLabel *checksTitle = new QLabel("Security Checks");
    checksTitle->setStyleSheet("color:#9cdcfe;font-size:13px;font-weight:bold;margin-top:8px;");
    mainLayout->addWidget(checksTitle);

    m_checksTable = new QTableWidget(0, 4);
    m_checksTable->setHorizontalHeaderLabels({"Check", "Result", "Score", "Recommendation"});
    m_checksTable->horizontalHeader()->setStretchLastSection(true);
    m_checksTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_checksTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_checksTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_checksTable->verticalHeader()->hide();
    m_checksTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_checksTable->setAlternatingRowColors(true);
    m_checksTable->setShowGrid(false);
    m_checksTable->setFont(QFont("Consolas", 9));
    mainLayout->addWidget(m_checksTable, 1);


    QLabel *summaryTitle = new QLabel("Summary");
    summaryTitle->setStyleSheet("color:#9cdcfe;font-size:13px;font-weight:bold;margin-top:4px;");
    mainLayout->addWidget(summaryTitle);

    m_summaryTable = new QTableWidget(0, 2);
    m_summaryTable->setHorizontalHeaderLabels({"Metric", "Value"});
    m_summaryTable->horizontalHeader()->setStretchLastSection(true);
    m_summaryTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_summaryTable->verticalHeader()->hide();
    m_summaryTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_summaryTable->setAlternatingRowColors(true);
    m_summaryTable->setShowGrid(false);
    m_summaryTable->setFont(QFont("Consolas", 9));
    m_summaryTable->setMaximumHeight(80);
    mainLayout->addWidget(m_summaryTable);
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

void SecurityAuditWidget::updateData(const fsi_audit_report_t *audit)
{
    if (!audit) return;


    m_scoreLabel->setText(QString::number(audit->score));
    m_gradeLabel->setText(QString("[%1]").arg(QString::fromLocal8Bit(audit->grade)));

    QColor gradeColor = (audit->score >= 75) ? QColor(78, 201, 176)
                      : (audit->score >= 50) ? QColor(220, 220, 170)
                      : QColor(244, 71, 71);
    m_gradeLabel->setStyleSheet(
        QString("color:%1;font-size:42px;font-weight:bold;font-family:Consolas;")
        .arg(gradeColor.name()));

    m_scoreBar->setValue(audit->score);


    m_checksTable->setRowCount(0);
    for (int i = 0; i < audit->check_count; i++) {
        const fsi_audit_check_t *c = &audit->checks[i];
        int row = m_checksTable->rowCount();
        m_checksTable->insertRow(row);

        m_checksTable->setItem(row, 0, new QTableWidgetItem(
            QString::fromLocal8Bit(c->name)));

        QTableWidgetItem *resItem = new QTableWidgetItem(
            QString::fromLocal8Bit(fsi_audit_result_str(c->result)));
        resItem->setForeground(resultColor(c->result));
        resItem->setFont(QFont("Consolas", 9, QFont::Bold));
        m_checksTable->setItem(row, 1, resItem);

        QString scoreStr = QString("%1/%2")
            .arg(c->score_contribution)
            .arg(c->max_contribution);
        m_checksTable->setItem(row, 2, new QTableWidgetItem(scoreStr));

        QString rec = QString::fromLocal8Bit(c->recommendation ? c->recommendation : "");
        if (rec.length() > 60) rec = rec.left(57) + "...";
        QTableWidgetItem *recItem = new QTableWidgetItem(rec);
        if (c->recommendation) {
            recItem->setToolTip(QString::fromLocal8Bit(c->recommendation));
        }
        m_checksTable->setItem(row, 3, recItem);
    }

    m_summaryTable->setRowCount(0);
    auto addRow = [this](const QString &field, const QString &value) {
        int row = m_summaryTable->rowCount();
        m_summaryTable->insertRow(row);
        m_summaryTable->setItem(row, 0, new QTableWidgetItem(field));
        m_summaryTable->setItem(row, 1, new QTableWidgetItem(value));
    };

    int pass = 0, warn = 0, fail = 0, info = 0;
    for (int i = 0; i < audit->check_count; i++) {
        switch (audit->checks[i].result) {
            case AUDIT_PASS: pass++; break;
            case AUDIT_WARNING: warn++; break;
            case AUDIT_FAIL: fail++; break;
            default: info++; break;
        }
    }

    addRow("Total Checks", QString::number(audit->check_count));
    addRow("Pass", QString::number(pass));
    addRow("Warnings", QString::number(warn));
    addRow("Fail", QString::number(fail));
    addRow("Info", QString::number(info));
}