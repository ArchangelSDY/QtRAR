QT       -= gui

TARGET = QtRAR
TEMPLATE = lib

DEFINES += QTRAR_LIBRARY _FILE_OFFSET_BITS=64 _LARGEFILE_SOURCE RAR_SMP RARDLL
CONFIG(staticlib): DEFINES += QTRAR_STATIC

include(src.pri)

unix {
    target.path = /usr/lib
    INSTALLS += target
}

coverage {
    QMAKE_CXXFLAGS += -g -Wall -fprofile-arcs -ftest-coverage -O0
    QMAKE_LFLAGS += -g -Wall -fprofile-arcs -ftest-coverage -O0
}
