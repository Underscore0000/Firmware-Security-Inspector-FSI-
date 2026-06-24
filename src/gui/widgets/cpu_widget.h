#ifndef CPU_WIDGET_H
#define CPU_WIDGET_H

#include <QWidget>
#include <QLabel>
#include <QTableWidget>
#include <QGroupBox>
#include <QTabWidget>
#include <QTreeWidget>

extern "C" {
#include "cpu.h"
#include "report.h"
}

class CpuWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CpuWidget(QWidget *parent = nullptr);
    void updateData(const fsi_cpu_info_t *cpu);

private:
    void setupUI();
    void populateGeneralTab(const fsi_cpu_info_t *cpu);
    void populateFeaturesTab(const fsi_cpu_info_t *cpu);
    void populateCacheTab(const fsi_cpu_info_t *cpu);
    void populateRawTab(const fsi_cpu_info_t *cpu);

    QTabWidget      *m_tabWidget;
    QTableWidget    *m_generalTable;
    QTableWidget    *m_featuresTable;
    QTableWidget    *m_cacheTable;
    QTableWidget    *m_rawTable;
    QLabel          *m_vendorLabel;
    QLabel          *m_brandLabel;
    QLabel          *m_microarchLabel;
};

#endif 