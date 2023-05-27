TARGET   = sessionviewer.plugin
TEMPLATE = lib
QT       += widgets

DESTDIR = $(BUILD_DIR)/lib/plugins/mappi.app.widgets

LOG_MODULE_NAME = sessionviewerplugin

HEADERS = sessionviewerplugin.h \
          sessionviewerwidget.h

SOURCES = sessionviewerplugin.cpp \
          sessionviewerwidget.cpp

FORMS =   sessionviewerwidget.ui \

LIBS    = -lmeteo.mainwindow          \
          -lmeteo.proto               \
          -ltdebug                    \
          -ltapp                      \
          -ltcustomui                 \
          -lmnfuncs                   \
          -lmnsatellite               \
          -lmeteo.global              \
          -lmappi.global              \
          -lmeteo.geobasis            \
          -lmeteo.map                 \
          -lmeteo.map.view            \
          -lmappi.settings            \
          -lsatlayer \
          -lmeteo.map.ptkpp

          
PROTOPATH += $(SRC)/meteo/commons/proto
include( $$(SRC)/protobuf.pri )
include( $$(SRC)/include.pro )

