#ifndef SMBIOS_WIDGET_H
#define SMBIOS_WIDGET_H

#include <QWidget>
#include <QTableWidget>

extern "C" {
#include "smbios.h"
}

class SmbiosWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SmbiosWidget(QWidget *parent = nullptr);
    void updateData(const fsi_smbios_info_t *smbios);

private:
    QTableWidget *m_table;
};

#endif