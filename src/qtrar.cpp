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
};

QtRARPrivate::QtRARPrivate(QtRAR *q) :
    m_q(q) ,
    m_mode(QtRAR::OpenModeNotOpen) ,
    m_error(ERAR_SUCCESS) ,
    m_hasScaned(false)
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
    m_hasScaned(false)
{
    m_dArc.ArcName = m_arcNameEncoded.data();
    m_dArc.CmtBuf = new char[QtRAR::MAX_COMMENT_SIZE];
    m_dArc.CmtBufSize = QtRAR::MAX_COMMENT_SIZE;
}

QtRARPrivate::~QtRARPrivate()
{
    delete m_dArc.CmtBuf;
}

void QtRARPrivate::scanFileInfo()
{
    // FIXME: after scan, current changed

    if (!m_q->isOpen()) {
        return;
    }

    m_fileInfoList.clear();

    QtRARFileInfo curInfo;
    if (!m_q->currentFileInfo(&curInfo)) {
        return;
    }

    QtRARFileInfo info;

    // To the end
    while (m_q->goToNextFile()) {
        if (m_q->currentFileInfo(&info)) {
            m_fileInfoList << info;
        }
    }

    // From the beginning
    if (!m_q->goToFirstFile()) {
        return;
    }

    do {
        if (m_q->currentFileInfo(&info)) {
            m_fileInfoList << info;
            if (info.fileName == curInfo.fileName) {
                break;
            }
        }
    } while (m_q->goToNextFile());

    m_hasScaned = true;
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

    bool isSuccess = (m_p->m_error == ERAR_SUCCESS);
    if (isSuccess) {
        m_p->m_mode = mode;
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
    if (!m_p->m_hasScaned) {
        m_p->scanFileInfo();
    }

    return m_p->m_fileInfoList.count();
}

bool QtRAR::goToFirstFile()
{
    if (!isOpen()) {
        return false;
    }

    // Reopen to reset the cursor.
    // Save mode first because mode will be reset after close().
    OpenMode lastOpenMode = m_p->m_mode;
    close();
    return open(lastOpenMode);
}

bool QtRAR::goToNextFile()
{
    if (!isOpen()) {
        return false;
    }

    m_p->m_error = RARProcessFile(m_p->m_hArc, RAR_SKIP, NULL, NULL);
    if (m_p->m_error != ERAR_SUCCESS) {
        return false;
    } else {
        RARHeaderData hData;
        return RARReadHeader(m_p->m_hArc, &hData) == ERAR_SUCCESS;
    }
}

bool QtRAR::setCurrentFile(const QString &fileName, Qt::CaseSensitivity cs)
{
    if (!isOpen()) {
        return false;
    }

    QHash<QString, bool> searched;
    QtRARFileInfo info;
    while (currentFileInfo(&info)) {
        if (info.fileName.compare(fileName, cs) == 0) {
            return true;
        } else {
            searched.insert(fileName, true);
            if (!goToNextFile()) {
                break;
            }
        }
    }

    // Not found until the end. Go to first and search again.
    if (goToFirstFile()) {
        while (currentFileInfo(&info)) {
            if (info.fileName.compare(fileName, cs) == 0) {
                return true;
            } else if (searched.contains(info.fileName)) {
                // Already checked starting from this
                return false;
            } else {
                if (!goToNextFile()) {
                    return false;
                }
            }
        }
    }

    return false;
}

bool QtRAR::hasCurrentFile() const
{
    if (!isOpen()) {
        return false;
    }

    RARHeaderData hData;
    return RARReadHeader(m_p->m_hArc, &hData) == ERAR_SUCCESS;
}

bool QtRAR::currentFileInfo(QtRARFileInfo *info) const
{
    if (!isOpen()) {
        return false;
    }

    RARHeaderData hData;
    int err = RARReadHeader(m_p->m_hArc, &hData);

    if (err == ERAR_SUCCESS) {
        info->fileName = QString::fromUtf8(hData.FileName);
        info->arcName = m_p->m_arcName;
        info->flags = hData.Flags;
        info->packSize = hData.PackSize;
        info->unpSize = hData.UnpSize;
        info->hostOS = hData.HostOS;
        info->fileCRC = hData.FileCRC;
        info->fileTime = hData.FileTime;
        info->unpVer = hData.UnpVer;
        info->method = hData.Method;
        info->fileAttr = hData.FileAttr;
        info->comment = QString::fromUtf8(m_p->m_dArc.CmtBuf,
                                          m_p->m_dArc.CmtSize);
        return true;
    } else {
        return false;
    }
}

QString QtRAR::currentFileName() const
{
    if (!isOpen()) {
        return QString();
    }

    RARHeaderData hData;
    int err = RARReadHeader(m_p->m_hArc, &hData);
    if (err == ERAR_SUCCESS) {
        return QString::fromUtf8(hData.FileName);
    } else {
        return QString();
    }
}

QStringList QtRAR::fileNameList() const
{
    if (!m_p->m_hasScaned) {
        m_p->scanFileInfo();
    }

    QStringList list;
    foreach (const QtRARFileInfo &info, m_p->m_fileInfoList) {
        list << info.fileName;
    }
    return list;
}

QList<QtRARFileInfo> QtRAR::fileInfoList() const
{
    if (!m_p->m_hasScaned) {
        m_p->scanFileInfo();
    }

    return m_p->m_fileInfoList;
}
