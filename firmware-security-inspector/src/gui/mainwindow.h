#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidget>
#include <QStackedWidget>
#include <QSplitter>
#include <QStatusBar>
#include <QMenuBar>
#include <QProgressBar>
#include <QLabel>
#include <QThread>

extern "C" {
#include "report.h"
#include "snapshot.h"
}


class DataCollectorThread : public QThread
{
    Q_OBJECT
public:
    explicit DataCollectorThread(QObject *parent = nullptr);
    fsi_system_snapshot_t *snapshot() { return &m_snap; }

protected:
    void run() override;

signals:
    void progressUpdate(int percent, const QString &msg);
    void collectionDone();

private:
    fsi_system_snapshot_t m_snap;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onNavItemSelected(QListWidgetItem *item);
    void onRefreshClicked();
    void onSaveSnapshot();
    void onLoadSnapshot();
    void onCompareSnapshots();
    void onExportReport();
    void onCollectionDone();
    void onProgressUpdate(int percent, const QString &msg);

private:
    void setupMenuBar();
    void setupLayout();
    void setupNavBar();
    void setupPages();
    void updateAllPages();
    void setStatus(const QString &msg);

    QSplitter          *m_splitter;
    QListWidget        *m_navList;
    QStackedWidget     *m_stack;
    QProgressBar       *m_progressBar;
    QLabel             *m_statusLabel;


    QWidget *m_pageDashboard;
    QWidget *m_pageFirmware;
    QWidget *m_pageCpu;
    QWidget *m_pageRegisters;
    QWidget *m_pageMsr;
    QWidget *m_pageSecureBoot;
    QWidget *m_pageTpm;
    QWidget *m_pageAcpi;
    QWidget *m_pageSmbios;
    QWidget *m_pagePcie;
    QWidget *m_pageMemory;
    QWidget *m_pageAudit;
    QWidget *m_pageReports;
    QWidget *m_pageDeveloper;

    DataCollectorThread      *m_collector;
    fsi_system_snapshot_t    *m_snapshot;
    bool                      m_dataReady;
};

#endif 