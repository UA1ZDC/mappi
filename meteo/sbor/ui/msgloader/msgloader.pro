TEMPLATE = app

TARGET = mappi.msgloader

LOG_MODULE_NAME = sbor

QT      =       core gui widgets

SOURCES =       main.cpp                \
                dataloader.cpp          \
                obanalstatus.cpp        \
                loader.cpp              \
                loaderstatus.cpp        \
                documentstatus.cpp      \
                climatstatus.cpp

HEADERS =       dataloader.h            \
                obanalstatus.h          \
                loader.h                \
                loaderstatus.h          \
                documentstatus.h        \
                climatstatus.h

FORMS   =       dataloader.ui           \
                obanalstatus.ui         \
                loaderstatus.ui         \
                documentstatus.ui       \
                climatstatus.ui

LIBS    =       -lmeteo.msgparser       \
                -lmeteo.settings        \
                -lmeteo.textproto       \
                -lmeteo.proto           \
                -lprotobuf              \
                -ltapp                  \
                -ltdebug                \
                -lmeteo.etc             \
                -lmeteo.global          \
                -lmeteo.dbi             \
                -lmeteo.punchrules      \
                -ltrpc


include( $$(SRC)/include.pro )
