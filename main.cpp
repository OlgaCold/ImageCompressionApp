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

    ImageHandler *lib = new ImageHandler();

    const std::string inputFilename = "../resources/test-image-2-gs.bmp";
    const std::string compressedFilename = "compressed_image.barch";
    const std::string restoredFilename = "restored_image.bmp";

    lib->compressImage(inputFilename, compressedFilename);

    lib->restoreImage(compressedFilename, restoredFilename);

    std::cout << "Testing complete." << std::endl;

    return 0;
    //return app.exec();
}
