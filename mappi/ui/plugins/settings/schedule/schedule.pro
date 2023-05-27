TARGET     = schedule
TEMPLATE   = lib
QT        += widgets

DESTDIR    = $(BUILD_DIR)/lib/plugins/mappi.app.widgets

SOURCES   +=  schedulesetting.cpp \
              schedulesettingwidget.cpp \
              satelliteeditor.cpp \
              receivereditor.cpp

HEADERS   +=  schedulesetting.h \
              schedulesettingwidget.h \
              satelliteeditor.h \
              receivereditor.h

FORMS     +=  schedulesettingwidget.ui \
              satelliteeditor.ui \
              receivereditor.ui

DISTFILES +=  schedule.json

LIBS      +=  -lsettings.base


PROTOPATH += $(SRC)/meteo/commons/proto
include( $$(SRC)/protobuf.pri )

include( $$(SRC)/include.pro )
