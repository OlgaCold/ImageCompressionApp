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

    //Compression *lib = new Compression();
    //lib->compress("test-image-1-gs.bmp");

    // Read an image from disk, modify it and write it back:
    //BMP bmp("../resources/test-image-1-gs.bmp");
    Compression *lib = new Compression();

    // Стиснення зображення
    const std::string compressedFilename = "compressed_image.barch";
    //lib->_readBMP("../resources/test-image-1-gs.bmp");
    lib->compressImage("../resources/test-image-2-gs.bmp");

    // Відновлення стислого зображення
    lib->restoreImage(compressedFilename);

    // Запис BMP файлу
    //std::string restoredFilename = "restored_image.bmp";

    std::cout << "Testing complete." << std::endl;

    return 0;
    //return app.exec();
}
