#ifndef QTRARFILE_H
#define QTRARFILE_H

#include <QIODevice>

#include "qtrar_global.h"

class QtRAR;
struct QtRARFileInfo;
class QtRARFilePrivate;

class QTRARSHARED_EXPORT QtRARFile : public QIODevice
{
    Q_OBJECT
    friend class QtRARFilePrivate;
public:
    explicit QtRARFile(QObject *parent = 0);
    explicit QtRARFile(const QString &arcName, QObject *parent = 0);
    explicit QtRARFile(const QString &arcName, const QString &fileName,
                       Qt::CaseSensitivity cs=Qt::CaseSensitive,
                       QObject *parent = 0);
    explicit QtRARFile(QtRAR *rar, QObject *parent = 0);
    virtual ~QtRARFile();

    QString arcName() const;
    QtRAR *rar() const;
    QString fileName() const;
    Qt::CaseSensitivity caseSensitivity() const;
    QString actualFileName() const;

    void setArchiveName(const QString &arcName);
    void setArchive(QtRAR *rar);
    void setFileName(const QString &fileName,
                     Qt::CaseSensitivity caseSensitivity=Qt::CaseSensitive);

    virtual bool open(OpenMode mode);
    bool open(OpenMode mode, const QString &password);
    virtual bool isSequential() const;
    virtual qint64 pos() const;
    virtual bool atEnd() const;
    virtual qint64 size() const;
    qint64 csize() const;
    qint64 usize() const;
    virtual qint64 bytesAvailable() const;
    virtual void close();

    bool fileInfo(QtRARFileInfo *info);
    int error() const;

protected:
    qint64 readData(char *data, qint64 maxlen);
    qint64 writeData(const char *data, qint64 len);

private:
    QtRARFilePrivate *m_p;
};

#endif // QTRARFILE_H
