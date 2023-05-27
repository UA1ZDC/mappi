TARGET = meteo.settings
TEMPLATE = lib

HEADERS = \
  settings.h \
  tusersettings.h

SOURCES = \
  settings.cpp \
  tusersettings.cpp

LIBS += \
  -ltdebug \
  -ltsingleton \
  -lmnfuncs\
  -lsql.proto


include( $$(SRC)/include.pro )
