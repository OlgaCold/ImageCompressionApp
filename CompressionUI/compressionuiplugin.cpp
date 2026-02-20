#include "filemodel.h"
#include <QtQml/qqmlextensionplugin.h>
#include <qqml.h>

class CompressionUIPlugin : public QQmlExtensionPlugin {
  Q_OBJECT
  Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)
public:
  void registerTypes(const char *uri) override {
    qmlRegisterType<FileModel>(uri, 1, 0, "FileModel");
  }
};

#include "compressionuiplugin.moc"