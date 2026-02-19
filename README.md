# ImageCompressionApp
C++ application for image compression/decompression.
Built with Qt 5 (qmake). Uses a modular architecture.

## Overview
The project consists of three main parts:
* __Core Library.__ Shared C++ library (libcompression.so) implementing compression logic.

* __QML Plugin.__ Custom Qt plugin exposing reusable QML components:
  * Data model
  * Custom button component
  * Dialog component

* __Application Layer.__ QML-based UI built on top of the plugin and linked against the shared library.

## Requirements
 - Ubuntu Linux
 - Qt version 5.x (tested with 5.12)
 - qmake
 - C++14 compatible compiler
 - GNU Make 4.2.1


## Setup on Clean Ubuntu
```bash
# Update package list
sudo apt update

# Install build tools
sudo apt install build-essential

# Install Qt (QtCore + QtQuick + QML support)
sudo apt install qtbase5-dev qtdeclarative5-dev \
                 qtchooser qt5-qmake qtbase5-dev-tools

# Verify
qmake --version
```

## Build
```bash
chmod +x build.sh
./build.sh

# or with optional input folder:
/build.sh <path_to_images>
```

## Build Artifacts
Generated in:
```bash
./build
```
Shared library:
```bash
build/compressionLib/libcompression.so
```
QML plugin:
```bash
build/plugin/
```
Application executable:
```bash
build/ImageCompressionApp
```