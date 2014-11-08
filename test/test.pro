QT       += testlib
QT       -= gui

TARGET = qtrartest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

QMAKE_MAC_SDK = macosx10.9

include(test.pri)

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/src/release/ -lQtRAR
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/src/debug/ -lQtRAR
else:unix: LIBS += -L$$OUT_PWD/src -lQtRAR

assets.files = $$PWD/assets/*.rar
assets.path = $$OUT_PWD

INSTALLS += assets

coverage {
    QMAKE_CXXFLAGS += -g -Wall -fprofile-arcs -ftest-coverage -O0
    QMAKE_LFLAGS += -g -Wall -fprofile-arcs -ftest-coverage -O0
}
