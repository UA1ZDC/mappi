TEMPLATE = lib
TARGET = mappi.antenna.widget

QT += \
  gui \
  widgets \
  serialport

HEADERS = \
  coordsystem.h \
  targetpoint.h \
  scene.h \
  view.h \
  navigator.h \
  clientstub.h \
  antennawidget.h

SOURCES = \
  coordsystem.cpp \
  targetpoint.cpp \
  scene.cpp \
  view.cpp \
  navigator.cpp \
  clientstub.cpp \
  antennawidget.cpp

FORMS = \
  antennawidget.ui

LIBS += \
  -L$(BUILD_DIR)/lib \
  -lprotobuf \
  -ltdebug \
  -ltapp \
  -lmnprocread \
  -lmnmathtools \
  -lmappi.antenna \
  -lmappi.proto

PROTOPATH += $(SRC)/meteo/commons/proto
include( $$(SRC)/protobuf.pri )

include( $$(SRC)/include.pro )
