TEMPLATE = app
TARGET   = mappi.msgstream

LOG_MODULE_NAME = recgmi

SOURCES =       main.cpp

LIBS    =       -lmeteo.msgstream.appmain       \
                -lmeteo.punchrules              \
                -ltalphanum
include( $$(SRC)/include.pro )

