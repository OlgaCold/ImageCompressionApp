#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDir>
#include <QCoreApplication>

int main(int argc, char **argv) {
    QGuiApplication app(argc, argv);
    QString startDir;
    if (argc > 1) {
        QString cand = QString::fromLocal8Bit(argv[1]);
        QDir d(cand);
        if (d.exists()) startDir = d.absolutePath();
    }

    if (startDir.isEmpty()) {
        QDir appDir(QCoreApplication::applicationDirPath());
        QString cand = appDir.filePath("resources");
        if (QDir(cand).exists()) {
            startDir = QDir(cand).absolutePath();
        } else {
            appDir.cdUp();
            cand = appDir.filePath("resources");
            if (QDir(cand).exists()) {
                startDir = QDir(cand).absolutePath();
            } else {
                startDir = QDir::currentPath();
            }
        }
    }

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("startDirectory", startDir);
    engine.load(QUrl(QStringLiteral("qrc:/qml/Main.qml")));
    if (engine.rootObjects().isEmpty()) return -1;
    return app.exec();
}
