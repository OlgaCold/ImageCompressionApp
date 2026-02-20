#include "filemodel.h"
#include <QFileInfo>
#include <QtConcurrent/QtConcurrent>
#include <QMetaObject>
#include <QDebug>
#include <string>

#include <compression.h>

FileModel::FileModel(QObject* parent) : QAbstractListModel(parent) {
    // current dir by default
    setDirectory(QDir::currentPath());
}

int FileModel::rowCount(const QModelIndex&) const {
    return m_items.size();
}

QVariant FileModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() < 0 || index.row() >= m_items.size()) return {};

    const auto &it = m_items.at(index.row());
    switch (role) {
        case NameRole: return it.name;
        case SizeRole: return it.size;
        case StatusRole: return it.status;
        case PathRole: return it.path;
        case ExtRole: return it.ext;
        default: return {};
    }
}

QHash<int, QByteArray> FileModel::roleNames() const {
    return { {NameRole,"name"}, {SizeRole,"size"}, {StatusRole,"status"}, {PathRole,"path"}, {ExtRole,"ext"} };
}

void FileModel::setDirectory(const QString& dir) {
    QDir newd(dir);
    if (!newd.exists()) newd = QDir::current();
    m_dir = newd;
    rescan();
    emit directoryChanged();
}

void FileModel::setExtensions(const QStringList& exts) {
    if (m_extensions != exts) {
        m_extensions = exts;
        rescan();  // with new filters
        emit extensionsChanged();
    }
}

void FileModel::rescan() {
    beginResetModel();
    m_items.clear();

    QFileInfoList list = m_dir.entryInfoList(m_extensions, QDir::Files | QDir::NoDotAndDotDot, QDir::Name);
    for (const QFileInfo &fi : list) {
        FileItem it;
        it.name = fi.fileName();
        it.path = fi.absoluteFilePath();
        it.size = fi.size();
        it.ext = fi.suffix().toLower();
        it.status.clear();
        m_items.append(it);
    }
    qDebug() << "FileModel::rescan" << m_dir.path() << "found" << m_items.size() << "items with filters:" << m_extensions;
    endResetModel();
}

void FileModel::activate(int index) {
    if (index < 0 || index >= m_items.size()) return;
    const QString path = m_items[index].path;
    const QString ext = m_items[index].ext;

    if (ext == "bmp" || ext == "barch") {
        m_items[index].status = (ext == "bmp") ? QStringLiteral("Encoding") : QStringLiteral("Decoding");
        QModelIndex mi = createIndex(index, 0);
        emit dataChanged(mi, mi, {StatusRole});

        QtConcurrent::run([this, index, path, ext]() {
            // start processing
            //setProcessStateChanged();
            int res = -1;
            QString outPath;
            try {
                ImageHandler handler;
                if (ext == "bmp") {
                    outPath = path + QStringLiteral(".packed.barch");
                    handler.compressImage(path.toStdString(), outPath.toStdString());
                    res = 0;
                } else {
                    outPath = path + QStringLiteral(".unpacked.bmp");
                    handler.restoreImage(path.toStdString(), outPath.toStdString());
                    res = 0;
                }
            } catch (const std::exception &e) {
                qWarning() << "Compression error:" << e.what();
                res = -1;
            } catch (...) {
                qWarning() << "Unknown compression error";
                res = -1;
            }

            QMetaObject::invokeMethod(this, [this, index, res]() {
                if (index < 0 || index >= m_items.size()) return;
                m_items[index].status = (res == 0) ? QString() : QStringLiteral("Error");
                QModelIndex mi2 = createIndex(index, 0);
                emit dataChanged(mi2, mi2, {StatusRole});
                rescan();  // to update list with new files after processing
                emit directoryChanged();
            }, Qt::QueuedConnection);
        });
        
    } else {
        emit errorRequested(QStringLiteral("Unknown file"));
    }
}
