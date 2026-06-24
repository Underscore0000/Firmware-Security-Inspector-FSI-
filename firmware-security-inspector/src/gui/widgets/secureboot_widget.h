#ifndef SECUREBOOT_WIDGET_H
#define SECUREBOOT_WIDGET_H

#include <QWidget>
#include <QTableWidget>

extern "C" {
#include "secureboot.h"
}

class SecureBootWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SecureBootWidget(QWidget *parent = nullptr);
    void updateData(const fsi_secureboot_info_t *sb);

private:
    QTableWidget *m_table;
};

#endif