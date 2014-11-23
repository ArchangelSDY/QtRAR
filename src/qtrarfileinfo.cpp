#include "qtrarfileinfo.h"

bool QtRARFileInfo::isEncrypted() const
{
    return flags & 0x04;
}
