QT -= core gui

TARGET = unrar
TEMPLATE = lib
CONFIG += staticlib

QMAKE_MAC_SDK = macosx10.9

DEFINES += _FILE_OFFSET_BITS=64 _LARGEFILE_SOURCE RAR_SMP RARDLL

include(unrar.pri)
