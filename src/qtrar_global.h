#ifndef QTRAR_GLOBAL_H
#define QTRAR_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(QTRAR_LIBRARY)
#  define QTRARSHARED_EXPORT Q_DECL_EXPORT
#else
#  define QTRARSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // QTRAR_GLOBAL_H
