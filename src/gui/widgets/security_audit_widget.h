#ifndef SECURITY_AUDIT_WIDGET_H
#define SECURITY_AUDIT_WIDGET_H

#include <QWidget>
#include <QTableWidget>
#include <QLabel>
#include <QProgressBar>
#include <QGroupBox>

extern "C" {
#include "security_audit.h"
#include "report.h"
}

class SecurityAuditWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SecurityAuditWidget(QWidget *parent = nullptr);
    void updateData(const fsi_audit_report_t *audit);

private:
    void setupUI();

    QLabel          *m_scoreLabel;
    QLabel          *m_gradeLabel;
    QProgressBar    *m_scoreBar;
    QTableWidget    *m_checksTable;
    QTableWidget    *m_summaryTable;
    QGroupBox       *m_scoreBox;
};

#endif 