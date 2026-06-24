#ifndef MEMORY_WIDGET_H
#define MEMORY_WIDGET_H

#include <QWidget>
#include <QTableWidget>

extern "C" {
#include "memory.h"
}

class MemoryWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MemoryWidget(QWidget *parent = nullptr);
    void updateData(const fsi_memory_map_t *memory);

private:
    QTableWidget *m_table;
};

#endif