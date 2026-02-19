#include <QtQml/qqmlextensionplugin.h>
#include <qqml.h>
#include "filemodel.h"

class MyQmlComponentsPlugin : public QQmlExtensionPlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)
public:
    void registerTypes(const char *uri) override {
        qmlRegisterType<FileModel>(uri, 1, 0, "FileModel");
    }
};

#include "myqmlcomponentsplugin.moc"