#ifndef QTRAR_H
#define QTRAR_H

#include <QStringList>
#include <Qt>

#include "qtrar_global.h"

class QtRARFile;
struct QtRARFileInfo;
class QtRARPrivate;

class QTRARSHARED_EXPORT QtRAR
{
    friend class QtRARPrivate;
public:
    static const int MAX_COMMENT_SIZE = 64 * 1024;
    static const int MAX_ARC_NAME_SIZE = 2048;

    enum OpenMode {
        OpenModeNotOpen,
        OpenModeList,
        OpenModeExtract,
    };

    QtRAR();
    QtRAR(const QString &arcName);
    ~QtRAR();

    // TODO codec

    bool open(OpenMode mode, const QString &password = QString());
    void close();
    bool isOpen() const;
    OpenMode mode() const;
    int error() const;

    QString archiveName() const;
    void setArchiveName(const QString &arcName);
    QString comment() const;
    int entriesCount() const;
    bool isHeadersEncrypted() const;
    bool isFilesEncrypted() const;

    bool setCurrentFile(const QString &fileName,
                        Qt::CaseSensitivity cs = Qt::CaseSensitive);

    bool currentFileInfo(QtRARFileInfo *info) const;
    QString currentFileName() const;

    QStringList fileNameList() const;
    QList<QtRARFileInfo> &fileInfoList() const;

    Qt::HANDLE unrarArcHandle();
    // TODO: auto close？

private:
    QtRAR(const QtRAR &that);
    QtRAR &operator=(const QtRAR &that);

    QtRARPrivate *m_p;

};

#endif // QTRAR_H
