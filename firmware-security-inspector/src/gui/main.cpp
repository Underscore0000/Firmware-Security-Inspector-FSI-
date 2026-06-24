#include <QApplication>
#include <QStyleFactory>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);


    app.setStyle(QStyleFactory::create("Fusion"));


    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window,          QColor(30, 30, 30));
    darkPalette.setColor(QPalette::WindowText,      QColor(212, 212, 212));
    darkPalette.setColor(QPalette::Base,            QColor(20, 20, 20));
    darkPalette.setColor(QPalette::AlternateBase,   QColor(37, 37, 38));
    darkPalette.setColor(QPalette::ToolTipBase,     QColor(25, 25, 25));
    darkPalette.setColor(QPalette::ToolTipText,     QColor(212, 212, 212));
    darkPalette.setColor(QPalette::Text,            QColor(212, 212, 212));
    darkPalette.setColor(QPalette::Button,          QColor(45, 45, 48));
    darkPalette.setColor(QPalette::ButtonText,      QColor(212, 212, 212));
    darkPalette.setColor(QPalette::BrightText,      Qt::red);
    darkPalette.setColor(QPalette::Link,            QColor(86, 156, 214));
    darkPalette.setColor(QPalette::Highlight,       QColor(38, 79, 120));
    darkPalette.setColor(QPalette::HighlightedText, QColor(255, 255, 255));
    darkPalette.setColor(QPalette::Disabled,
                         QPalette::Text, QColor(100, 100, 100));
    darkPalette.setColor(QPalette::Disabled,
                         QPalette::ButtonText, QColor(100, 100, 100));

    app.setPalette(darkPalette);
    app.setFont(QFont("Consolas", 9));

    MainWindow w;
    w.setWindowTitle("Firmware Security Inspector v1.0");
    w.resize(1280, 800);
    w.show();

    return app.exec();
}