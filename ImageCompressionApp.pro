QT += quick qml quickcontrols2 concurrent
CONFIG += c++11
TEMPLATE = app

SOURCES += main.cpp

RESOURCES += qml.qrc

INCLUDEPATH += $$PWD/compressionLib \
               $$PWD/CompressionUI

LIBS += -L$$PWD/compressionLib -lcompression \
        -L$$PWD/build/CompressionUI -lCompressionUIPlugin
