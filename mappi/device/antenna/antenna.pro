TEMPLATE = lib
TARGET = mappi.antenna

QT += \
  serialport

HEADERS = \
  drive.h \
  feedhorn.h \
  profile.h \
  config.h \
  protocol.h \
  vkaprotocol.h \
  niitvprotocol.h \
  anglespeedprotocol.h \
  transport.h \
  track.h \
  script.h \
  direct.h \
  catcher.h \
  tracker.h \
  monkeytest.h \
  scriptchain.h \
  antenna.h

SOURCES = \
  drive.cpp \
  feedhorn.cpp \
  profile.cpp \
  config.cpp \
  vkaprotocol.cpp \
  niitvprotocol.cpp \
  anglespeedprotocol.cpp \
  transport.cpp \
  track.cpp \
  script.cpp \
  direct.cpp \
  catcher.cpp \
  tracker.cpp \
  monkeytest.cpp \
  scriptchain.cpp \
  antenna.cpp

PROTOPATH += $(SRC)/meteo/commons/proto
include( $$(SRC)/protobuf.pri )

include( $$(SRC)/include.pro )
