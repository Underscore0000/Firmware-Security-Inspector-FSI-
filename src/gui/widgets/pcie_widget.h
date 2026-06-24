#ifndef PCIE_WIDGET_H
#define PCIE_WIDGET_H

#include <QWidget>
#include <QTableWidget>

extern "C" {
#include "pcie.h"
}

class PcieWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PcieWidget(QWidget *parent = nullptr);
    void updateData(const fsi_pcie_data_t *pcie);

private:
    QTableWidget *m_table;
};

#endif