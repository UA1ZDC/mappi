TEMPLATE = lib
TARGET   = meteo.reserve
QT += widgets

SOURCES =  datasaver.cpp    \
           reservewidget.cpp\
           appmain.cpp

HEADERS =  datasaver.h      \
           appmain.h        \
           reservewidget.h

FORMS   =  reserve.ui


LIBS    = -ltapp                    \
          -ltdebug                  \
          -ltrpc                    \
          -lmeteo.settings          \
          -lmeteo.global            \
          -ltsingleton              \
          -lprotobuf                \
          -lmeteo.sql                \
          -lmeteo.textproto         \
          -lmeteo.proto             \
          -lmeteo.map               \
          -lmeteo.map               \
          -lmeteo.map.dataexchange  \
          -ltcustomui               \
          -lmeteo.mainwindow        \
          -lmeteo.map.view

include( $$(SRC)/include.pro )
