#pragma once
#include <QAbstractListModel>
#include <QDir>

// compressionLib API (C++ functions)
extern void compressImage(const std::string& inputFilename, const std::string& outputFilename);
extern void restoreImage(const std::string& compressedFilename, const std::string& restoredFilename);

struct FileItem { QString name; QString path; qint64 size=0; QString status; QString ext; };

class FileModel : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(QString directory READ directory WRITE setDirectory NOTIFY directoryChanged)
public:
    enum Roles { NameRole = Qt::UserRole+1, SizeRole, StatusRole, PathRole, ExtRole };
    explicit FileModel(QObject* parent=nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    QString directory() const { return m_dir.path(); }

public slots:
    void setDirectory(const QString& dir);
    Q_INVOKABLE void activate(int index);

signals:
    void directoryChanged();
    void errorRequested(const QString &message);

private:
    void rescan();
    QVector<FileItem> m_items;
    QDir m_dir;
};