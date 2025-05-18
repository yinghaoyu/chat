#include "mainwindow.h"
#include <QApplication>
#include <QPropertyAnimation>
#include <QFile>
#include "global.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QFile qss(":/style/stylesheet.qss");

    if( qss.open(QFile::ReadOnly))
    {
        qDebug("open success");
        QString style = QLatin1String(qss.readAll());
        a.setStyleSheet(style);
        qss.close();
    }else{
         qDebug("Open failed");
     }


    // 获取当前应用程序的路径
    QString app_path = QCoreApplication::applicationDirPath();
    // 拼接文件名
    QString fileName = "config.ini";
    QString config_path = QDir::toNativeSeparators(app_path +
                             QDir::separator() + fileName);

    QSettings settings(config_path, QSettings::IniFormat);
    QString gate_host = settings.value("GateServer/host").toString();
    QString gate_port = settings.value("GateServer/port").toString();
    gate_url_prefix = "http://"+gate_host+":"+gate_port;

    // 创建一个动画效果
    // QPropertyAnimation *animation = new QPropertyAnimation(button, "geometry");
    // animation->setDuration(1000); // 动画持续时间，单位毫秒
    // animation->setStartValue(QRect(50, 50, 100, 40)); // 动画开始位置
    // animation->setEndValue(QRect(150, 100, 100, 40)); // 动画结束位置

    // 启动动画
    //animation->start();

    MainWindow w;
    w.show();
    return a.exec();
}
