#!/bin/bash
set -euo pipefail
IFS=$'\n\t'

BASEDIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="$BASEDIR/build"
NPROC=$(nproc 2>/dev/null || echo 1)
# Path to installed QT
QT_PATH="/opt/Qt/5.12.8/gcc_64"

echo "=== Build script started ==="
echo "Base dir: $BASEDIR"
echo "Build dir: $BUILD_DIR"
echo "Parallel jobs: $NPROC"

mkdir -p "$BUILD_DIR"
mkdir -p "$BUILD_DIR/compressionLib"
mkdir -p "$BUILD_DIR/MyQmlComponents"

# 1) Build compressionLib -> build/compressionLib
COMP_SRC="$BASEDIR/compressionLib"
COMP_BUILD="$BUILD_DIR/compressionLib"
if [ -d "$COMP_SRC" ]; then
  echo "--- Building compressionLib -> $COMP_BUILD ---"
  pushd "$COMP_BUILD" >/dev/null
  qmake "$COMP_SRC/compressionLib.pro" || { echo "qmake failed for compressionLib" >&2; popd >/dev/null; exit 1; }
  make -j"$NPROC" || { echo "make failed for compressionLib" >&2; popd >/dev/null; exit 1; }
  popd >/dev/null
else
  echo "Error: compressionLib not found at $COMP_SRC" >&2
  exit 1
fi

# 2) Build MyQmlComponents -> build/MyQmlComponents
QML_SRC="$BASEDIR/MyQmlComponents"
QML_BUILD="$BUILD_DIR/MyQmlComponents"
if [ -d "$QML_SRC" ]; then
  echo "--- Building MyQmlComponents -> $QML_BUILD ---"
  pushd "$QML_BUILD" >/dev/null
  qmake "$QML_SRC/MyQmlComponents.pro" || { echo "qmake failed for MyQmlComponents" >&2; popd >/dev/null; exit 1; }
  make -j"$NPROC" || { echo "make failed for MyQmlComponents" >&2; popd >/dev/null; exit 1; }
  popd >/dev/null

  # ensure qmldir next to plugin
  if [ -f "$QML_SRC/qmldir" ]; then
    cp -f "$QML_SRC/qmldir" "$QML_BUILD/" || true
  else
    cat > "$QML_BUILD/qmldir" <<EOF
module MyQmlComponents
plugin MyQmlComponentsPlugin
EOF
  fi
else
  echo "Error: MyQmlComponents not found at $QML_SRC" >&2
  exit 1
fi

# 3) Build main app -> build
echo "--- Building main application -> $BUILD_DIR ---"
pushd "$BUILD_DIR" >/dev/null
qmake "$BASEDIR/ImageCompressionApp.pro" || { echo "qmake failed for app" >&2; popd >/dev/null; exit 1; }
make -j"$NPROC" || { echo "make failed for app" >&2; popd >/dev/null; exit 1; }
popd >/dev/null

echo "Main application built in $BUILD_DIR"

export LD_LIBRARY_PATH="$QT_PATH/lib:$BUILD_DIR:${LD_LIBRARY_PATH:-}"
export QML2_IMPORT_PATH="$QT_PATH/qml:$BUILD_DIR:${QML2_IMPORT_PATH:-}"
echo "LD_LIBRARY_PATH=$LD_LIBRARY_PATH"
echo "QML2_IMPORT_PATH=$QML2_IMPORT_PATH"

APP_EXE="$BUILD_DIR/ImageCompressionApp"
if [ -x "$APP_EXE" ]; then
  echo "--- Running application ---"
  "$APP_EXE" "${@}"
else
  echo "Executable not found at $APP_EXE" >&2
  exit 1
fi

echo "=== Build script finished ==="