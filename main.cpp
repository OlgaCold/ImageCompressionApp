#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDir>
#include <QCoreApplication>
#include "MyQmlComponents/filemodel.h"

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QGuiApplication app(argc, argv);

    QString dir;
    if (argc > 1) {
        dir = QString::fromLocal8Bit(argv[1]);
    } else {
        QString appDir = QCoreApplication::applicationDirPath();
        QDir candidate(appDir);
        candidate.cdUp();
        candidate.cd("resources");
        if (candidate.exists())
            dir = candidate.absolutePath();
        else
            dir = QDir::currentPath();
    }

    FileModel model;
    model.setDirectory(dir);
    //model.setExtensions(QStringList{"*.bmp", "*.barch", "*.png"});

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("fileModel", &model);
    engine.load(QUrl(QStringLiteral("qrc:/MyQmlComponents/qml/Main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
