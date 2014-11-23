#include <QDebug>
#include <QHash>

#include "unrar/raros.hpp"
#include "unrar/dll.hpp"
#undef HANDLE

#include "qtrar.h"
#include "qtrarfileinfo.h"

class QtRARPrivate
{
    friend class QtRAR;
public:
    inline QtRARPrivate(QtRAR *q);
    inline QtRARPrivate(QtRAR *q, const QString &arcName);
    inline ~QtRARPrivate();

private:
    void reset();
    bool reopen();
    void scanFileInfo();

    QtRAR *m_q;
    QtRAR::OpenMode m_mode;
    int m_error;
    QString m_arcName;
    Qt::HANDLE m_hArc;
    QString m_comment;
    bool m_isHeadersEncrypted;
    QByteArray m_password;

    bool m_hasScaned;
    QList<QtRARFileInfo> m_fileInfoList;
    QHash<QString, int> m_fileNameToIndexSensitive;
    QHash<QString, int> m_fileNameToIndexInsensitive;
    int m_curIndex;
};

QtRARPrivate::QtRARPrivate(QtRAR *q) :
    m_q(q) ,
    m_mode(QtRAR::OpenModeNotOpen) ,
    m_error(ERAR_SUCCESS) ,
    m_isHeadersEncrypted(false) ,
    m_hasScaned(false) ,
    m_curIndex(0)
{
}

QtRARPrivate::QtRARPrivate(QtRAR *q, const QString &arcName) :
    m_q(q) ,
    m_mode(QtRAR::OpenModeNotOpen) ,
    m_error(ERAR_SUCCESS) ,
    m_arcName(arcName) ,
    m_isHeadersEncrypted(false) ,
    m_hasScaned(false) ,
    m_curIndex(0)
{
}

QtRARPrivate::~QtRARPrivate()
{
}

void QtRARPrivate::reset()
{
    m_fileInfoList.clear();
    m_hasScaned = false;
    m_isHeadersEncrypted = false;
    m_password.clear();
    m_curIndex = 0;
    m_error = ERAR_SUCCESS;
    m_hArc = 0;
    m_mode = QtRAR::OpenModeNotOpen;
}

bool QtRARPrivate::reopen()
{
    QtRAR::OpenMode lastOpenMode = m_mode;
    m_q->close();
    return m_q->open(lastOpenMode);
}

void QtRARPrivate::scanFileInfo()
{
    if (!m_q->isOpen() || m_hasScaned) {
        return;
    }

    m_fileInfoList.clear();

    RARHeaderData hData;
    int i = 0;
    while (RARReadHeader(m_hArc, &hData) == ERAR_SUCCESS) {
        QtRARFileInfo info;

        info.fileName = QString::fromUtf8(hData.FileName);
        info.arcName = m_arcName;
        info.flags = hData.Flags;
        info.packSize = hData.PackSize;
        info.unpSize = hData.UnpSize;
        info.hostOS = hData.HostOS;
        info.fileCRC = hData.FileCRC;
        info.fileTime = hData.FileTime;
        info.unpVer = hData.UnpVer;
        info.method = hData.Method;
        info.fileAttr = hData.FileAttr;
        info.comment = m_q->comment();

        m_fileInfoList << info;
        m_fileNameToIndexSensitive.insert(info.fileName, i);
        m_fileNameToIndexInsensitive.insert(info.fileName.toLower(), i);
        i++;

        if (RARProcessFile(m_hArc, RAR_SKIP, NULL, NULL) != ERAR_SUCCESS) {
            break;
        }
    };

    m_hasScaned = true;

    // Reopen to reset cursor
    reopen();
}


QtRAR::QtRAR() :
    m_p(new QtRARPrivate(this))
{
}

QtRAR::QtRAR(const QString &arcName) :
    m_p(new QtRARPrivate(this, arcName))
{
}

QtRAR::~QtRAR()
{
    if (isOpen()) {
        close();
    }
    delete m_p;
}

bool QtRAR::open(OpenMode mode, const QString &password)
{
    RAROpenArchiveDataEx arcData;
    wchar_t arcNameW[MAX_ARC_NAME_SIZE];
    int arcNameLen = m_p->m_arcName
            .left(MAX_ARC_NAME_SIZE - 1)
            .toWCharArray(arcNameW);
    arcNameW[arcNameLen] = '\0';
    arcData.ArcNameW = arcNameW;
    arcData.ArcName = 0;
    arcData.CmtBuf = new char[MAX_COMMENT_SIZE];
    arcData.CmtBufSize = MAX_COMMENT_SIZE;
    arcData.Callback = 0;
    arcData.UserData = 0;

    if (mode == OpenModeList) {
        arcData.OpenMode = RAR_OM_LIST;
    } else {
        arcData.OpenMode = RAR_OM_EXTRACT;
    }

    m_p->m_hArc = RAROpenArchiveEx(&arcData);
    m_p->m_error = arcData.OpenResult;
    // Comment buffer ends with '\0'
    m_p->m_comment = QString::fromUtf8(arcData.CmtBuf, arcData.CmtSize - 1);
    m_p->m_isHeadersEncrypted = (arcData.Flags & 0x0080);
    m_p->m_curIndex = 0;

    bool isSuccess = (m_p->m_error == ERAR_SUCCESS);
    if (isSuccess) {
        m_p->m_mode = mode;

        if (!password.isEmpty()) {
            m_p->m_password = password.toUtf8();
            RARSetPassword(m_p->m_hArc, m_p->m_password.data());
        }

        m_p->scanFileInfo();
    } else {
        m_p->m_mode = OpenModeNotOpen;
    }

    delete arcData.CmtBuf;
    return isSuccess;
}

void QtRAR::close()
{
    if (isOpen()) {
        RARCloseArchive(m_p->m_hArc);
        m_p->m_hArc = 0;
        m_p->m_mode = OpenModeNotOpen;
        m_p->m_curIndex = 0;
    }
}

bool QtRAR::isOpen() const
{
    return m_p->m_mode != OpenModeNotOpen;
}

QtRAR::OpenMode QtRAR::mode() const
{
    return m_p->m_mode;
}

int QtRAR::error() const
{
    return m_p->m_error;
}

QString QtRAR::archiveName() const
{
    return m_p->m_arcName;
}

void QtRAR::setArchiveName(const QString &arcName)
{
    if (isOpen()) {
        qWarning() << "QtRAR::setArchiveName: Archive is open now! Close it first.";
        return;
    }

    if (arcName != m_p->m_arcName) {
        m_p->reset();
    }

    m_p->m_arcName = arcName;
}

QString QtRAR::comment() const
{
    return m_p->m_comment;
}

int QtRAR::entriesCount() const
{
    return m_p->m_fileInfoList.count();
}

bool QtRAR::isHeadersEncrypted() const
{
    return m_p->m_isHeadersEncrypted;
}

bool QtRAR::setCurrentFile(const QString &fileName, Qt::CaseSensitivity cs)
{
    if (!isOpen()) {
        return false;
    }

    // Move unrar cursor to this index
    if (!m_p->reopen()) {
        qWarning() << "QtRAR::setCurrentFile: fail to reopen to reset cursor";
        return false;
    }

    QHash<QString, int>::const_iterator it;
    if (cs == Qt::CaseSensitive) {
        it = m_p->m_fileNameToIndexSensitive.find(fileName);
        if (it == m_p->m_fileNameToIndexSensitive.end()) {
            return false;
        }
    } else {
        it = m_p->m_fileNameToIndexInsensitive.find(fileName.toLower());
        if (it == m_p->m_fileNameToIndexInsensitive.end()) {
            return false;
        }
    }

    m_p->m_curIndex = it.value();

    for (int i = 0; i < m_p->m_curIndex; ++i) {
        RARHeaderData hData;
        if (RARReadHeader(m_p->m_hArc, &hData) == ERAR_SUCCESS) {
            if (RARProcessFile(m_p->m_hArc, RAR_SKIP, 0, 0) == ERAR_SUCCESS) {
                continue;
            } else {
                qWarning() << "QtRAR::setCurrentFile: fail to skip file at index"
                           << i;
            }
        } else {
            qWarning() << "QtRAR:setCurrentFile: fail to read head at index"
                       << i;
            return false;
        }
    }

    return true;
}

bool QtRAR::currentFileInfo(QtRARFileInfo *info) const
{
    if (!isOpen()) {
        return false;
    }

   *info = m_p->m_fileInfoList[m_p->m_curIndex];
    return true;
}

QString QtRAR::currentFileName() const
{
    if (!isOpen()) {
        return QString();
    }

    return m_p->m_fileInfoList[m_p->m_curIndex].fileName;
}

QStringList QtRAR::fileNameList() const
{
    QStringList list;
    foreach (const QtRARFileInfo &info, m_p->m_fileInfoList) {
        list << info.fileName;
    }
    return list;
}

QList<QtRARFileInfo> &QtRAR::fileInfoList() const
{
    return m_p->m_fileInfoList;
}

Qt::HANDLE QtRAR::unrarArcHandle()
{
    return m_p->m_hArc;
}
