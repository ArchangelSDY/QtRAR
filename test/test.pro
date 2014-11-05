QT       += testlib
QT       -= gui

TARGET = qtrartest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

QMAKE_MAC_SDK = macosx10.9

include(test.pri)

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/src/release/ -lqtrar
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/src/debug/ -lqtrar
else:unix: LIBS += -L$$OUT_PWD/src -lqtrar

assets.files = $$PWD/assets/*.rar
assets.path = $$OUT_PWD

INSTALLS += assets
