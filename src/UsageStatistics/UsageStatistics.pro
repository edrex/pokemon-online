#-------------------------------------------------
#
# Project created by QtCreator 2010-07-22T23:36:49
#
#-------------------------------------------------

TARGET = usagestats
DESTDIR = ../../bin/myplugins
TEMPLATE = lib

DEFINES += POKEMONONLINESTATSPLUGIN_LIBRARY
DEFINES += SERVER_SIDE
LIBS += -L../../bin \
    -lmaindll

SOURCES += usagestats.cpp

HEADERS += usagestats.h\
        usagestats_global.h
