#ifndef HEX_VIEWER_H
#define HEX_VIEWER_H

#include <QWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>

class HexViewer : public QWidget
{
    Q_OBJECT
public:
    explicit HexViewer(QWidget *parent = nullptr);
    void setData(const uint8_t *data, size_t size, const QString &title = QString());
    void clear();

private slots:
    void onOffsetChanged(const QString &text);
    void onFindChanged(const QString &text);
    void onGoToClicked();

private:
    void setupUI();
    void renderHex();
    size_t findBytes(const QString &pattern, size_t start = 0);
    QString formatHex(const uint8_t *data, size_t size, size_t start, size_t end);

    const uint8_t *m_data;
    size_t         m_size;
    QString        m_title;

    QTextEdit      *m_hexEdit;
    QLineEdit      *m_offsetEdit;
    QLineEdit      *m_findEdit;
    QPushButton    *m_goToBtn;
    QLabel         *m_statusLabel;
    size_t          m_currentOffset;
};

#endif 