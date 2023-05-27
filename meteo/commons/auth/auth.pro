TEMPLATE = lib

TARGET = meteo.auth

HEADERS += \
  gecos.h \
  engine.h \
  pam_context.h \
  pam.h \
  ald_stub.h\
  cache.h \
  auth.h

SOURCES += \
  gecos.cpp \
  engine.cpp \
  pam_context.cpp \
  pam.cpp \
  ald_stub.cpp \
  cache.cpp \
  auth.cpp

LIBS = \
  -lpam

include( $$(SRC)/include.pro )

defined( T_OS_ASTRA_LINUX, var ) {
  HEADERS += ald.h
  HEADERS -= ald_stub.cpp

  SOURCES += ald.cpp
  SOURCES -= ald_stub.cpp

  LIBS += -lald-core
}
