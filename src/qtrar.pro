QT       -= gui

TARGET = QtRAR
TEMPLATE = lib

DEFINES += QTRAR_LIBRARY

QMAKE_MAC_SDK = macosx10.9

CONFIG += ordered
SUBDIRS = unrar

LIBS += -L$$PWD/unrar -lunrar

include(qtrar.pri)

unix {
    target.path = /usr/lib
    INSTALLS += target
}
