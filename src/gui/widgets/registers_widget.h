#ifndef REGISTERS_WIDGET_H
#define REGISTERS_WIDGET_H

#include <QWidget>
#include <QTableWidget>

extern "C" {
#include "registers.h"
}

class RegistersWidget : public QWidget
{
    Q_OBJECT
public:
    explicit RegistersWidget(QWidget *parent = nullptr);
    void updateData(const fsi_registers_t *regs);

private:
    QTableWidget *m_table;
};

#endif