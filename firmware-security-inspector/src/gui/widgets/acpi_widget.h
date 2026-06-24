#ifndef ACPI_WIDGET_H
#define ACPI_WIDGET_H

#include <QWidget>
#include <QTableWidget>
#include <QTreeWidget>
#include <QTabWidget>
#include <QLabel>

extern "C" {
#include "acpi.h"
#include "report.h"
}

class AcpiWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AcpiWidget(QWidget *parent = nullptr);
    void updateData(const fsi_acpi_data_t *acpi);

private:
    void setupUI();
    void populateTablesTab(const fsi_acpi_data_t *acpi);
    void populateSummaryTab(const fsi_acpi_data_t *acpi);

    QTabWidget      *m_tabWidget;
    QTableWidget    *m_tablesTable;
    QTableWidget    *m_summaryTable;
    QLabel          *m_oemLabel;
    QLabel          *m_countLabel;
    QLabel          *m_statusLabel;
};

#endif 