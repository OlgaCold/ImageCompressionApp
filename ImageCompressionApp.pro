QT += quick

HEADERS += $$PWD/compressionLib/compression.h

SOURCES += main.cpp

RESOURCES += qml.qrc

INCLUDEPATH += $$PWD/compressionLib
LIBS += -L$$PWD/compressionLib -lcompression

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
