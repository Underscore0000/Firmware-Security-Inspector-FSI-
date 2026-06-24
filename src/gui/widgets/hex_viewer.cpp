#include "hex_viewer.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFont>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QScrollBar>
#include <QTextCursor>
#include <QMessageBox>
#include <QClipboard>
#include <QApplication>

HexViewer::HexViewer(QWidget *parent)
    : QWidget(parent)
    , m_data(nullptr)
    , m_size(0)
    , m_currentOffset(0)
{
    setupUI();
}

void HexViewer::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(12, 12, 12, 12);
    mainLayout->setSpacing(6);

    // Header
    QLabel *title = new QLabel("Hex Viewer");
    title->setStyleSheet(
        "color:#569cd6;font-size:16px;font-weight:bold;"
        "border-bottom:1px solid #3f3f46;padding-bottom:4px;");
    mainLayout->addWidget(title);

    // Toolbar
    QHBoxLayout *toolbar = new QHBoxLayout;

    QLabel *offsetLabel = new QLabel("Offset:");
    offsetLabel->setStyleSheet("color:#858585;");
    m_offsetEdit = new QLineEdit;
    m_offsetEdit->setPlaceholderText("0x0");
    m_offsetEdit->setFixedWidth(120);
    m_offsetEdit->setStyleSheet(
        "QLineEdit{background:#252526;color:#d4d4d4;border:1px solid #3f3f46;"
        "padding:2px 6px;border-radius:2px;}");
    connect(m_offsetEdit, &QLineEdit::returnPressed, this, &HexViewer::onGoToClicked);

    m_goToBtn = new QPushButton("Go");
    m_goToBtn->setFixedWidth(40);
    m_goToBtn->setStyleSheet(
        "QPushButton{background:#37373d;color:#d4d4d4;"
        "border:1px solid #555;padding:2px 6px;border-radius:2px;}"
        "QPushButton:hover{background:#4a4a52;}");
    connect(m_goToBtn, &QPushButton::clicked, this, &HexViewer::onGoToClicked);

    QLabel *findLabel = new QLabel("Find:");
    findLabel->setStyleSheet("color:#858585;margin-left:12px;");
    m_findEdit = new QLineEdit;
    m_findEdit->setPlaceholderText("hex bytes (e.g. FF 00 AA)");
    m_findEdit->setFixedWidth(200);
    m_findEdit->setStyleSheet(m_offsetEdit->styleSheet());
    connect(m_findEdit, &QLineEdit::textChanged, this, &HexViewer::onFindChanged);

    m_statusLabel = new QLabel("Ready");
    m_statusLabel->setStyleSheet("color:#858585;font-size:11px;");

    toolbar->addWidget(offsetLabel);
    toolbar->addWidget(m_offsetEdit);
    toolbar->addWidget(m_goToBtn);
    toolbar->addWidget(findLabel);
    toolbar->addWidget(m_findEdit);
    toolbar->addStretch();
    toolbar->addWidget(m_statusLabel);
    mainLayout->addLayout(toolbar);

    // Hex text area
    m_hexEdit = new QTextEdit;
    m_hexEdit->setReadOnly(true);
    m_hexEdit->setFont(QFont("Consolas", 10));
    m_hexEdit->setStyleSheet(
        "QTextEdit{background:#1a1a1a;color:#d4d4d4;border:1px solid #3f3f46;"
        "font-family:Consolas;font-size:10px;"
        "selection-background-color:#094771;}");
    mainLayout->addWidget(m_hexEdit);
}

void HexViewer::setData(const uint8_t *data, size_t size, const QString &title)
{
    m_data = data;
    m_size = size;
    m_title = title;
    m_currentOffset = 0;
    m_offsetEdit->clear();

    if (data && size > 0) {
        m_statusLabel->setText(QString("Size: %1 bytes (%2 KB)")
            .arg(size).arg(size / 1024.0, 0, 'f', 2));
        renderHex();
    } else {
        clear();
    }

    if (!title.isEmpty()) {
        QLabel *titleLabel = findChild<QLabel*>();
        if (titleLabel) {
            titleLabel->setText("Hex Viewer - " + title);
        }
    }
}

void HexViewer::clear()
{
    m_hexEdit->clear();
    m_statusLabel->setText("No data loaded");
}

void HexViewer::renderHex()
{
    if (!m_data || m_size == 0) {
        m_hexEdit->clear();
        return;
    }

    QString hex = formatHex(m_data, m_size, m_currentOffset, m_size);
    m_hexEdit->setPlainText(hex);


    int lineHeight = QFontMetrics(m_hexEdit->font()).lineSpacing();
    int line = m_currentOffset / 16;
    m_hexEdit->verticalScrollBar()->setValue(line * lineHeight);
}

QString HexViewer::formatHex(const uint8_t *data, size_t size, size_t start, size_t end)
{
    QString result;
    const size_t MAX_LINES = 10000;
    size_t lines = 0;

    for (size_t offset = start; offset < end && lines < MAX_LINES; offset += 16) {
        if (lines > 0) result += "\n";

        // Offset
        result += QString("%1  ").arg(offset, 8, 16, QChar('0')).toUpper();

        // Hex bytes
        for (size_t i = 0; i < 16; i++) {
            size_t idx = offset + i;
            if (idx < size) {
                result += QString("%1 ").arg(data[idx], 2, 16, QChar('0')).toUpper();
            } else {
                result += "   ";
            }
            if (i == 7) result += " ";
        }

        result += "  ";

        for (size_t i = 0; i < 16; i++) {
            size_t idx = offset + i;
            if (idx < size) {
                unsigned char c = data[idx];
                if (c >= 32 && c <= 126) {
                    result += QChar(c);
                } else {
                    result += '.';
                }
            } else {
                result += ' ';
            }
        }

        lines++;
    }

    if (end - start > MAX_LINES * 16) {
        result += QString("\n... truncated (showing %1 of %2 bytes)")
            .arg(MAX_LINES * 16).arg(size - start);
    }

    return result;
}

void HexViewer::onOffsetChanged(const QString &text)
{
    
}

void HexViewer::onFindChanged(const QString &text)
{
    if (text.isEmpty() || !m_data || m_size == 0) {
        renderHex();
        return;
    }


    QByteArray hexData = text.toLatin1();
    QList<QByteArray> parts = hexData.split(' ');
    QByteArray searchBytes;
    for (const QByteArray &part : parts) {
        if (part.isEmpty()) continue;
        bool ok;
        uint8_t val = part.toUInt(&ok, 16);
        if (ok) {
            searchBytes.append((char)val);
        }
    }

    if (searchBytes.isEmpty()) {
        renderHex();
        return;
    }


    size_t pos = findBytes(searchBytes.toHex(), 0);
    if (pos != (size_t)-1) {
        m_currentOffset = pos;
        renderHex();
        m_statusLabel->setText(QString("Found at offset 0x%1").arg(pos, 8, 16, QChar('0')));
    } else {
        m_statusLabel->setText("Pattern not found");
    }
}

size_t HexViewer::findBytes(const QString &pattern, size_t start)
{
    if (!m_data || m_size == 0 || pattern.isEmpty()) return (size_t)-1;

    QByteArray search = QByteArray::fromHex(pattern.toLatin1());
    if (search.isEmpty()) return (size_t)-1;

    for (size_t i = start; i <= m_size - (size_t)search.size(); i++) {
        if (memcmp(m_data + i, search.constData(), (size_t)search.size()) == 0) {
            return i;
        }
    }
    return (size_t)-1;
}

void HexViewer::onGoToClicked()
{
    if (!m_data || m_size == 0) return;

    QString text = m_offsetEdit->text().trimmed();
    if (text.isEmpty()) return;

    bool ok;
    size_t offset = text.toUInt(&ok, 0);
    if (!ok) {
        QMessageBox::warning(this, "Invalid Offset", "Enter a valid offset (e.g. 0x1000 or 4096)");
        return;
    }

    if (offset >= m_size) {
        QMessageBox::warning(this, "Offset Out of Range",
            QString("Offset 0x%1 exceeds data size 0x%2")
            .arg(offset, 0, 16).arg(m_size, 0, 16));
        return;
    }

    m_currentOffset = offset;
    renderHex();
    m_statusLabel->setText(QString("Jumped to offset 0x%1").arg(offset, 8, 16, QChar('0')));
}