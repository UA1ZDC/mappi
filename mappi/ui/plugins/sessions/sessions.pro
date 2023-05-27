TARGET     = sessions.plugin
TEMPLATE   = lib
QT        += widgets

DESTDIR    = $(BUILD_DIR)/lib/plugins/mappi.app.widgets

HEADERS   += \
    satellitetrackerscene.h \
    receiverimagescene.h \
    sessionswidget.h \
    sessions.h \
    receiverimageview.h

SOURCES   += \
    satellitetrackerscene.cpp \
    receiverimagescene.cpp \
    sessionswidget.cpp \
    sessions.cpp \
    receiverimageview.cpp

FORMS     += \
    sessionswidget.ui

DISTFILES += \
    sessions.json

LIBS      += \
    -lmeteo.map.oldgeo         \
    -lmeteo.map.ptkpp         \
    -lmappi.pos                \
    -lmappi.fileservice.client \
    -lmappi.scheduletable

RESOURCES  = $$(SRC)/mappi/etc/mappi.qrc


PROTOPATH += $(SRC)/meteo/commons/proto
include( $$(SRC)/protobuf.pri )

include( $$(SRC)/include.pro )

