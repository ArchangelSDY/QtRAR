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
    bool reopen();
    void scanFileInfo();

    QtRAR *m_q;
    QtRAR::OpenMode m_mode;
    int m_error;
    QString m_arcName;
    QByteArray m_arcNameEncoded;
    Qt::HANDLE m_hArc;
    RAROpenArchiveData m_dArc;

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
    m_hasScaned(false) ,
    m_curIndex(0)
{
    m_dArc.CmtBuf = new char[QtRAR::MAX_COMMENT_SIZE];
    m_dArc.CmtBufSize = QtRAR::MAX_COMMENT_SIZE;
}

QtRARPrivate::QtRARPrivate(QtRAR *q, const QString &arcName) :
    m_q(q) ,
    m_mode(QtRAR::OpenModeNotOpen) ,
    m_error(ERAR_SUCCESS) ,
    m_arcName(arcName) ,
    m_arcNameEncoded(m_arcName.toUtf8()) ,
    m_hasScaned(false) ,
    m_curIndex(0)
{
    m_dArc.ArcName = m_arcNameEncoded.data();
    m_dArc.CmtBuf = new char[QtRAR::MAX_COMMENT_SIZE];
    m_dArc.CmtBufSize = QtRAR::MAX_COMMENT_SIZE;
}

QtRARPrivate::~QtRARPrivate()
{
    delete m_dArc.CmtBuf;
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
        info.comment = QString::fromUtf8(m_dArc.CmtBuf, m_dArc.CmtSize);

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

bool QtRAR::open(OpenMode mode)
{
    if (mode == OpenModeList) {
        m_p->m_dArc.OpenMode = RAR_OM_LIST;
    } else {
        m_p->m_dArc.OpenMode = RAR_OM_EXTRACT;
    }

    m_p->m_hArc = RAROpenArchive(&(m_p->m_dArc));
    m_p->m_error = m_p->m_dArc.OpenResult;
    m_p->m_curIndex = 0;

    bool isSuccess = (m_p->m_error == ERAR_SUCCESS);
    if (isSuccess) {
        m_p->m_mode = mode;

        m_p->scanFileInfo();
    } else {
        m_p->m_mode = OpenModeNotOpen;
    }

    return isSuccess;
}

void QtRAR::close()
{
    if (isOpen()) {
        RARCloseArchive(m_p->m_hArc);
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
        qWarning("QtRAR::setArchiveName: Archive is open now! Close it first.");
        return;
    }

    if (arcName != m_p->m_arcName) {
        m_p->m_fileInfoList.clear();
        m_p->m_hasScaned = false;
    }

    m_p->m_arcName = arcName;
    m_p->m_arcNameEncoded = arcName.toUtf8();
    m_p->m_dArc.ArcName = m_p->m_arcNameEncoded.data();
}

QString QtRAR::comment() const
{
    if (!isOpen()) {
        return QString();
    }

    return QString::fromUtf8(m_p->m_dArc.CmtBuf, m_p->m_dArc.CmtSize);
}

int QtRAR::entriesCount() const
{
    return m_p->m_fileInfoList.count();
}

bool QtRAR::setCurrentFile(const QString &fileName, Qt::CaseSensitivity cs)
{
    if (!isOpen()) {
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

QList<QtRARFileInfo> QtRAR::fileInfoList() const
{
    return m_p->m_fileInfoList;
}
