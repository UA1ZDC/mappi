TEMPLATE = lib
TARGET   = mappi.satprojection

QT += widgets

LOG_MODULE_NAME = map
PUB_HEADERS =           pos.h
HEADERS =     $$PUB_HEADERS  $$PRIVATE_HEADERS \
  #posgrid_doc.h

SOURCES =               pos.cpp
 # posgrid_doc.cpp

LIBS +=                -lmnsatellite

include( $$(SRC)/include.pro )
