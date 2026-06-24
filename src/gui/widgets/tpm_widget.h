#ifndef TPM_WIDGET_H
#define TPM_WIDGET_H

#include <QWidget>
#include <QTableWidget>

extern "C" {
#include "tpm.h"
}

class TpmWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TpmWidget(QWidget *parent = nullptr);
    void updateData(const fsi_tpm_info_t *tpm);

private:
    QTableWidget *m_table;
};

#endif