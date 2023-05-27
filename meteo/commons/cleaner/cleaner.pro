TEMPLATE = lib
TARGET = meteo.cleaner

INCLUDEPATH += \
  $$[QT_INSTALL_HEADERS]/QtGui/$$[QT_VERSION] \
  $$[QT_INSTALL_HEADERS]/QtCore/$$[QT_VERSION]

HEADERS = \
  param.h \
  archutil.h \
  removetask.h \
  appmain.h

SOURCES = \
  archutil.cpp \
  removetask.cpp \
  appmain.cpp

LIBS += \
  -ltapp \
  -ltrpc \
  -ltdebug \
  -lmnfuncs \
  -lprotobuf \
  -lmeteo.sql \
  -lmeteo.nosql \
  -lmeteo.global \
  -lmeteo.planner \
  -lmeteo.settings \
  -lmeteo.textproto \
  -lmeteo.proto

include( $$(SRC)/include.pro )
