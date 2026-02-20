QT += qml quick concurrent
CONFIG += plugin c++11
TEMPLATE = lib
TARGET = CompressionUIPlugin

SOURCES += compressionuiplugin.cpp \
           filemodel.cpp

HEADERS += filemodel.h

# include compression lib headers (source layout: compressionLib/)
INCLUDEPATH += $$PWD/../compressionLib

# Link against compression library built into build/compressionLib
# When qmake is run from build/MyQmlComponents, $$OUT_PWD points to that build dir,
# so ../compressionLib resolves to build/compressionLib
LIBS += -L$$OUT_PWD/../compressionLib -lcompression

# At runtime the plugin will look for libcompression; add rpath so loader finds it in build/compressionLib
QMAKE_RPATHDIR += $$OUT_PWD/../compressionLib