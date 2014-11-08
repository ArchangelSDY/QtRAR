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

coverage {
    QMAKE_CXXFLAGS += -g -Wall -fprofile-arcs -ftest-coverage -O0
    QMAKE_LFLAGS += -g -Wall -fprofile-arcs -ftest-coverage -O0
}
