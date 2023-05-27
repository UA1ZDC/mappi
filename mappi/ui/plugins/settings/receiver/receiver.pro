QT        += widgets
TEMPLATE   = lib
TARGET     = receiver

DESTDIR    = $(BUILD_DIR)/lib/plugins/mappi.app.widgets

SOURCES   +=  receiversetting.cpp       \
              receiversettingwidget.cpp

HEADERS   +=  receiversetting.h         \
              receiversettingwidget.h

FORMS     +=  receiversettingwidget.ui

DISTFILES +=  receiver.json

LIBS      += -lsettings.base

PROTOPATH += $(SRC)/meteo/commons/proto
include( $$(SRC)/protobuf.pri )

include( $$(SRC)/include.pro )
