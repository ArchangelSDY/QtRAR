QT       -= gui

TARGET = QtRAR
TEMPLATE = lib

DEFINES += QTRAR_LIBRARY
CONFIG(staticlib): DEFINES += QTRAR_STATIC

QMAKE_MAC_SDK = macosx10.9

include(src.pri)

unix {
    target.path = /usr/lib
    INSTALLS += target
}
