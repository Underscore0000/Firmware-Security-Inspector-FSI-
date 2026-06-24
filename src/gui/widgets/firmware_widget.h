#ifndef FIRMWARE_WIDGET_H
#define FIRMWARE_WIDGET_H

#include <QWidget>
#include <QTableWidget>

extern "C" {
#include "firmware.h"
}

class FirmwareWidget : public QWidget
{
    Q_OBJECT
public:
    explicit FirmwareWidget(QWidget *parent = nullptr);
    void updateData(const fsi_firmware_info_t *firmware);

private:
    QTableWidget *m_table;
};

#endif