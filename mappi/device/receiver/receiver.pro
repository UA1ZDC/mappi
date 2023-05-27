TEMPLATE = lib
TARGET = mappi.receiver

QT += \
  network

HEADERS = \
  apch.h \
  parameters.h \
  config.h \
  engine.h \
  udpengine.h \
  dumpengine.h \
  subprocessengine.h \
  receiver.h

SOURCES = \
  apch.cpp \
  parameters.cpp \
  config.cpp \
  udpengine.cpp \
  dumpengine.cpp \
  subprocessengine.cpp \
  receiver.cpp

OTHER_FILES = python/*.py

PROTOPATH += $(SRC)/meteo/commons/proto
include( $$(SRC)/protobuf.pri )

include( $$(SRC)/include.pro )

py.files = $$OTHER_FILES
py.path  = $$(BUILD_DIR)/include.py
!win32 {etc.extra = chmod a+x $$OTHER_FILES}
INSTALLS += py
