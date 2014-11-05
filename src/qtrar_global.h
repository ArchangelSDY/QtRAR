#ifndef QTRAR_GLOBAL_H
#define QTRAR_GLOBAL_H

#include <QtCore/qglobal.h>

#ifdef QTRAR_STATIC

#define QTRARSHARED_EXPORT

#else

#if defined(QTRAR_LIBRARY)
#  define QTRARSHARED_EXPORT Q_DECL_EXPORT
#else
#  define QTRARSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif    // QTRAR_STATIC

#endif // QTRAR_GLOBAL_H
