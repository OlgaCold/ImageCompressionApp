#include "CompressionUI/filemodel.h"
#include <QCoreApplication>
#include <QDir>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <iostream>

int main(int argc, char *argv[]) {
  QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

  QGuiApplication app(argc, argv);

  QString dir;
  bool dirValid = false;

  if (argc > 1) {
    QString argPath = QString::fromLocal8Bit(argv[1]);
    if (QDir(argPath).exists()) {
      dir = argPath;
      dirValid = true;
    } else {
      std::cerr << "Warning: Folder does not exist: " << argPath.toStdString()
                << ". Falling back to defaults." << std::endl;
    }
  }

  if (!dirValid) {
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
  // model.setExtensions(QStringList{"*.bmp", "*.barch", "*.png"});

  QQmlApplicationEngine engine;
  engine.rootContext()->setContextProperty("fileModel", &model);
  engine.load(QUrl(QStringLiteral("qrc:/CompressionUI/qml/Main.qml")));
  if (engine.rootObjects().isEmpty())
    return -1;

  return app.exec();
}
