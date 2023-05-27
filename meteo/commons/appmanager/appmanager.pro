TEMPLATE = lib
TARGET = meteo.app.manager
QT += network

DEFINES += DAEMON_USER=vgmdaemon

LOG_MODULE_NAME = contrdiag

HEADERS = \
  tappconf.h \
  tappcontrol.h \
  applogwatcher.h \
  authhandler.h \
  tappcontrolservice.h \
  tappcontrolservicemethodbag.h \
  usersettings.h \
  appmain.h

SOURCES = \
  tappconf.cpp \
  tappcontrol.cpp \
  applogwatcher.cpp \
  authhandler.cpp \
  tappcontrolservice.cpp \
  tappcontrolservicemethodbag.cpp \
  usersettings.cpp \
  appmain.cpp

LIBS += \
  -lprotobuf \
  -ltdebug \
  -ltapp \
  -ltrpc \
  -lprotobuf \
  -lmnprocread \
  -lcommons.proto \
  -lmeteo.global \
  -lmeteo.settings \
  -lmeteo.nosql \
  -lmeteo.proto \
  -lmeteo.textproto \
  -lmeteo.auth \

INCLUDEPATH += $$(SRC)/meteo/commons/proto \

include( $$(SRC)/include.pro )

defined( T_OS_ASTRA_LINUX, var ) {
LIBS += \
  -lparsec-mac \
  -lparsec-cap \
  -lparsec-base
}
