QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = mappi.animate

TEMPLATE = app

HEADERS += animate.hpp mainwindow.h

SOURCES += main.cpp mainwindow.cpp

FORMS +=  mainwindow.ui

LIBS +=  -ltapp                     \
         -ltdebug                   \
         -lmeteo.global             \
         -lmeteo.dbi

PROTOPATH +=    $(SRC)/meteo/commons/proto

include( $$(SRC)/protobuf.pri )
include($$(SRC)/include.pro)

QMAKE_CXXFLAGS += -Wno-error=class-memaccess
