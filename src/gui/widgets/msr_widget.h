#ifndef MSR_WIDGET_H
#define MSR_WIDGET_H

#include <QWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>

extern "C" {
#include "msr.h"
}

class MsrWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MsrWidget(QWidget *parent = nullptr);
    void updateData(const fsi_msr_data_t *msr);

private slots:
    void onFilterChanged(const QString &text);
    void onRefreshClicked();

private:
    void setupUI();
    void populateTable(const fsi_msr_data_t *msr);

    QTableWidget    *m_table;
    QLineEdit       *m_filterEdit;
    QPushButton     *m_refreshBtn;
    QLabel          *m_statusLabel;
    const fsi_msr_data_t *m_currentData;
};

#endif 