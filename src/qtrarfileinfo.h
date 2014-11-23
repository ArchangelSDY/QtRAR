#ifndef QTRARFILEINFO_H
#define QTRARFILEINFO_H

#include <QString>

#include "qtrar_global.h"

struct QTRARSHARED_EXPORT QtRARFileInfo
{
    QString fileName;
    QString arcName;
    unsigned int flags;
    unsigned int packSize;
    unsigned int unpSize;
    unsigned int hostOS;
    unsigned int fileCRC;
    unsigned int fileTime;
    unsigned int unpVer;
    unsigned int method;
    unsigned int fileAttr;
    QString comment;

    bool isEncrypted() const;
};

#endif // QTRARFILEINFO_H
