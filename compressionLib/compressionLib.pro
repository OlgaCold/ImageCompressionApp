TEMPLATE = lib
CONFIG += shared c++11 unversioned_libname unversioned_soname
QT -= gui

TARGET = compression

INCLUDEPATH += $$PWD

SOURCES += compression.cpp
HEADERS += compression.h