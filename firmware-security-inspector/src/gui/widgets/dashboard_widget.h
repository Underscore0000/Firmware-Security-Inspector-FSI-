#ifndef DASHBOARD_WIDGET_H
#define DASHBOARD_WIDGET_H

#include <QWidget>
#include <QLabel>
#include <QTableWidget>
#include <QGroupBox>

extern "C" {
#include "report.h"
}

class DashboardWidget : public QWidget
{
    Q_OBJECT
public:
    explicit DashboardWidget(QWidget *parent = nullptr);
    void updateData(const fsi_system_snapshot_t *snap);

private:
    void setupUI();

    QLabel       *m_scoreLabel;
    QLabel       *m_gradeLabel;
    QTableWidget *m_checksTable;
    QTableWidget *m_systemTable;
    QTableWidget *m_cpuTable;
    QGroupBox    *m_scoreBox;
};

#endif 