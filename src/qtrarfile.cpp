#include <QBuffer>
#include <QDebug>

#include "unrar/rar.hpp"
// Avoid conflict name with Qt::HANDLE
#undef HANDLE

#include "qtrar.h"
#include "qtrarfile.h"
#include "qtrarfileinfo.h"

class QtRARFilePrivate
{
    friend class QtRARFile;
public:
    explicit QtRARFilePrivate(QtRARFile *q);
    explicit QtRARFilePrivate(QtRARFile *q, const QString &arcName);
    explicit QtRARFilePrivate(QtRARFile *q, const QString &arcName,
                              const QString &fileName, Qt::CaseSensitivity cs);
    explicit QtRARFilePrivate(QtRARFile *q, QtRAR *rar);
    ~QtRARFilePrivate();

private:
    static int CALLBACK procCallback(UINT msg, LPARAM self, LPARAM addr, LPARAM size);
    void resetBuffer();

    QtRARFile *m_q;
    QString m_fileName;
    Qt::CaseSensitivity m_caseSensitivity;
    QtRAR *m_rar;
    bool m_isRARInternal;
    int m_error;
    QBuffer m_buffer;
    QtRARFileInfo m_info;
    QByteArray m_password;
};

QtRARFilePrivate::QtRARFilePrivate(QtRARFile *q) :
    m_q(q) ,
    m_caseSensitivity(Qt::CaseSensitive) ,
    m_rar(nullptr) ,
    m_isRARInternal(true) ,
    m_error(ERAR_SUCCESS)
{
}

QtRARFilePrivate::QtRARFilePrivate(QtRARFile *q, const QString &arcName) :
    m_q(q) ,
    m_caseSensitivity(Qt::CaseSensitive) ,
    m_rar(new QtRAR(arcName)) ,
    m_isRARInternal(true) ,
    m_error(ERAR_SUCCESS)
{
}

QtRARFilePrivate::QtRARFilePrivate(QtRARFile *q,
                                   const QString &arcName,
                                   const QString &fileName,
                                   Qt::CaseSensitivity cs) :
    m_q(q) ,
    m_fileName(fileName) ,
    m_caseSensitivity(cs) ,
    m_rar(new QtRAR(arcName)) ,
    m_isRARInternal(true) ,
    m_error(ERAR_SUCCESS)
{
}

QtRARFilePrivate::QtRARFilePrivate(QtRARFile *q, QtRAR *rar) :
    m_q(q) ,
    m_caseSensitivity(Qt::CaseSensitive) ,
    m_rar(rar) ,
    m_isRARInternal(false) ,
    m_error(ERAR_SUCCESS)
{
}

QtRARFilePrivate::~QtRARFilePrivate()
{
    if (m_rar && m_isRARInternal) {
        delete m_rar;
    }
}

int QtRARFilePrivate::procCallback(UINT msg, LPARAM rawSelf,
                                   LPARAM p1, LPARAM p2)
{
    QtRARFilePrivate *self = reinterpret_cast<QtRARFilePrivate *>(rawSelf);

    if (msg == UCM_PROCESSDATA) {
        const char *data = reinterpret_cast<const char *>(p1);
        qint64 size = p2;
        self->m_buffer.write(data, size);
    } else if (msg == UCM_NEEDPASSWORD) {
        char *passBuf = reinterpret_cast<char *>(p1);
        qint64 passBufSize = p2;
        strncpy(passBuf, self->m_password.data(),
                size_t(qMin(qint64(self->m_password.count()), passBufSize)));
    }

    return 1;
}

void QtRARFilePrivate::resetBuffer()
{
    m_buffer.buffer().clear();
    m_buffer.seek(0);
}


QtRARFile::QtRARFile(QObject *parent) :
    QIODevice(parent) ,
    m_p(new QtRARFilePrivate(this))
{
}

QtRARFile::QtRARFile(const QString &arcName, QObject *parent) :
    QIODevice(parent) ,
    m_p(new QtRARFilePrivate(this, arcName))
{
}

QtRARFile::QtRARFile(const QString &arcName, const QString &fileName,
                     Qt::CaseSensitivity cs, QObject *parent) :
    QIODevice(parent) ,
    m_p(new QtRARFilePrivate(this, arcName, fileName, cs))
{
}

QtRARFile::QtRARFile(QtRAR *rar, QObject *parent) :
    QIODevice(parent) ,
    m_p(new QtRARFilePrivate(this, rar))
{
}

QtRARFile::~QtRARFile()
{
    if (isOpen()) {
        close();
    }
    delete m_p;
}

QString QtRARFile::arcName() const
{
    return m_p->m_rar ? m_p->m_rar->archiveName() : QString();
}

QtRAR *QtRARFile::rar() const
{
    return m_p->m_isRARInternal ? nullptr : m_p->m_rar;
}

QString QtRARFile::fileName() const
{
    return m_p->m_fileName;
}

Qt::CaseSensitivity QtRARFile::caseSensitivity() const
{
    return m_p->m_caseSensitivity;
}

QString QtRARFile::actualFileName() const
{
    return m_p->m_info.fileName;
}

void QtRARFile::setArchiveName(const QString &arcName)
{
    if (isOpen()) {
        qWarning() << "QtRARFile::setArchiveName: fail because current archive is opened";
        return;
    }

    if (m_p->m_rar && m_p->m_isRARInternal) {
        delete m_p->m_rar;
    }

    m_p->m_rar = new QtRAR(arcName);
    m_p->m_isRARInternal = true;
}

void QtRARFile::setArchive(QtRAR *rar)
{
    if (isOpen()) {
        qWarning() << "QtRARFile::setArchive: fail because current archive is opened";
        return;
    }

    if (m_p->m_rar && m_p->m_isRARInternal) {
        delete m_p->m_rar;
    }

    m_p->m_rar = rar;
    m_p->m_isRARInternal = false;
}

void QtRARFile::setFileName(const QString &fileName,
                            Qt::CaseSensitivity caseSensitivity)
{
    if (isOpen()) {
        qWarning() << "QtRARFile::setArchive: fail because current archive is opened";
        return;
    }

    m_p->m_fileName = fileName;
    m_p->m_caseSensitivity = caseSensitivity;
}

bool QtRARFile::open(OpenMode mode)
{
    return open(mode, nullptr);
}

bool QtRARFile::open(OpenMode mode, const QString &password)
{
    if (isOpen()) {
        qWarning() << "QtRARFile::open: already opened";
        return false;
    }

    if (mode != ReadOnly) {
        qWarning() << "QtRARFile::open: only support ReadOnly OpenMode";
        return false;
    }

    if (m_p->m_rar == nullptr) {
        qWarning() << "QtRARFile::open: archive is null";
        return false;
    }

    m_p->m_error = ERAR_SUCCESS;

    // Close QtRAR first in order to reopen with password if necessary
    if (m_p->m_rar->isOpen()) {
        m_p->m_rar->close();
    }

    if (!m_p->m_rar->open(QtRAR::OpenModeExtract, password)) {
        m_p->m_error = m_p->m_rar->error();
        return false;
    }

    m_p->m_password = password.toUtf8();

    if (!m_p->m_rar->setCurrentFile(m_p->m_fileName, m_p->m_caseSensitivity)) {
        qWarning() << "QtRARFile::open: fail to set current file to"
                   << m_p->m_fileName;
        return false;
    }

    if (!m_p->m_rar->currentFileInfo(&(m_p->m_info))) {
        qWarning() << "QtRARFile::open: fail to get current file info";
        return false;
    }

    RARSetCallback(m_p->m_rar->unrarArcHandle(),
                   QtRARFilePrivate::procCallback,
                   reinterpret_cast<LPARAM>(m_p));

    if (!m_p->m_buffer.isOpen()) {
        if (!m_p->m_buffer.open(ReadWrite)) {
            qWarning() << "QtRARFile::open: fail to open buffer";
            return false;
        }
    }

    RARHeaderData hData;
    if (RARReadHeader(m_p->m_rar->unrarArcHandle(), &hData) != ERAR_SUCCESS) {
        qWarning() << "QtRARFile::open: cannot read file meta info";
        return false;
    }

    m_p->resetBuffer();
    m_p->m_error = RARProcessFile(m_p->m_rar->unrarArcHandle(), RAR_TEST, nullptr, nullptr);
    m_p->m_buffer.seek(0);

    if (m_p->m_error == ERAR_SUCCESS) {
        return QIODevice::open(ReadOnly);
    } else {
        setOpenMode(NotOpen);
        return false;
    }
}

bool QtRARFile::isSequential() const
{
    return true;
}

qint64 QtRARFile::pos() const
{
    return m_p->m_buffer.pos();
}

bool QtRARFile::atEnd() const
{
    return m_p->m_buffer.atEnd();
}

qint64 QtRARFile::size() const
{
    return m_p->m_buffer.size();
}

qint64 QtRARFile::csize() const
{
    return m_p->m_info.packSize;
}

qint64 QtRARFile::usize() const
{
    return m_p->m_info.unpSize;
}

qint64 QtRARFile::bytesAvailable() const
{
    return m_p->m_buffer.bytesAvailable();
}

void QtRARFile::close()
{
    if (!isOpen()) {
        return;
    }

    if (m_p->m_rar && m_p->m_isRARInternal) {
        m_p->m_rar->close();
    }

    m_p->resetBuffer();
    setOpenMode(NotOpen);
}

int QtRARFile::error() const
{
    return m_p->m_error;
}

bool QtRARFile::fileInfo(QtRARFileInfo *info)
{
    if (m_p->m_rar) {
        return m_p->m_rar->currentFileInfo(info);
    } else {
        return false;
    }
}

qint64 QtRARFile::readData(char *data, qint64 maxlen)
{
    return m_p->m_buffer.read(data, maxlen);
}

qint64 QtRARFile::writeData(const char *, qint64)
{
    qWarning() << "QtRARFile::writeData: modify archive is not supported";
    return -1;
}
