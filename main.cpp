#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include "compressionLib/compression.h"

int main(/*int argc, char *argv[]*/)
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    //QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    //QGuiApplication app(argc, argv);

    //QQmlApplicationEngine engine;
    //const QUrl url(QStringLiteral("qrc:/main.qml"));
    //QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
    //                 &app, [url](QObject *obj, const QUrl &objUrl) {
    //    if (!obj && url == objUrl)
    //        QCoreApplication::exit(-1);
    //}, Qt::QueuedConnection);
    //engine.load(url);

    Compression *lib = new Compression();

    const std::string compressedFilename = "compressed_image.barch";
    //lib->_readBMP("../resources/test-image-1-gs.bmp");
    lib->compressImage("../resources/test-image-2-gs.bmp", compressedFilename);

    //lib->restoreImage(compressedFilename);

    //std::string restoredFilename = "restored_image.bmp";

    std::cout << "Testing complete." << std::endl;

    return 0;
    //return app.exec();
}
