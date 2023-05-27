TEMPLATE = lib
TARGET = meteo.admin
QT += widgets

PUB_HEADERS = usersettings.h

LOG_MODULE_NAME = settings

HEADERS = \
  dictmodel.h \
  dictview.h \
  usermodel.h \
  usersettings.h \
  userprofile.h \
  appmain.h

SOURCES =  \
  dictmodel.cpp \
  dictview.cpp \
  usermodel.cpp \
  usersettings.cpp \
  userprofile.cpp \
  appmain.cpp

FORMS = \
  usersettings.ui \
  dictview.ui \
  userprofile.ui

LIBS += \
  -ltapp \
  -ltdebug \
  -lmeteo.proto \
  -ltcustomui \
  -lmeteo.settings \
  -lmeteo.etc \
  -lmeteo.global \
  -lmeteo.map \
  -lmeteo.map.dataexchange \
  -lmeteo.map.view \
  -lmeteo.auth \
  -lmeteo.mainwindow

include( $$(SRC)/include.pro )
