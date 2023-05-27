TEMPLATE = lib
TARGET   = meteo.mainwindow.main

QT += widgets

LOG_MODULE_NAME = vizdoc

HEADERS =     startapp.h                \
              appmain.h

SOURCES =     appmain.cpp               \
              startapp.cpp

LIBS +=       -ltdebug                  \
              -ltapp                    \
              -lmeteo.sql                \
              -lprotobuf

LIBS +=       -lmeteo.proto             \
              -lmeteo.global            \
              -lmeteo.settings          \
              -lmeteo.mainwindow        \
              -lmeteo.tileimage

LIBS +=       -lmeteo.map               \
              -lmeteo.map.dataexchange  \
              -lmeteo.map.view          \
              -lmeteo.tileimage          \
              -lmeteo.etc


include( $$(SRC)/include.pro )

defined( T_OS_ASTRA_LINUX, var ) {
LIBS +=       -lparsec-mac              \
              -lparsec-cap              \
              -lparsec-base
}
