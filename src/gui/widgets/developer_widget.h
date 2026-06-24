#ifndef DEVELOPER_WIDGET_H
#define DEVELOPER_WIDGET_H

#include <QWidget>
#include <QTableWidget>
#include <QTreeWidget>
#include <QTabWidget>
#include <QLabel>
#include <QTextEdit>

extern "C" {
#include "report.h"
#include "cpu.h"
#include "msr.h"
#include "acpi.h"
#include "registers.h"
}

class DeveloperWidget : public QWidget
{
    Q_OBJECT
public:
    explicit DeveloperWidget(QWidget *parent = nullptr);
    void updateData(const fsi_system_snapshot_t *snap);

private:
    void setupUI();
    void populateCpuRawTab(const fsi_cpu_info_t *cpu);
    void populateRegisterTab(const fsi_registers_t *regs);
    void populateHexTab(const fsi_system_snapshot_t *snap);

    QTabWidget      *m_tabWidget;
    QTableWidget    *m_cpuRawTable;
    QTableWidget    *m_registerTable;
    QTextEdit       *m_hexEdit;
    QLabel          *m_statusLabel;
};

#endif