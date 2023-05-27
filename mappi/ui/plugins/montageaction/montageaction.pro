TEMPLATE = lib
TARGET   = mappi.montage
DESTDIR = $(BUILD_DIR)/lib/plugins/mappi.map.viewer

QT += widgets

LOG_MODULE_NAME = montage

HEADERS =   montageaction.h \
            thematiclistwidget.h \
            montageplugin.h \
            areascene.h

SOURCES =   montageaction.cpp \
            thematiclistwidget.cpp \
            montageplugin.cpp \
            areascene.cpp

FORMS =     thematiclistwidget.ui

OTHER_FILES =     montageplugin.json

LIBS =    -ltdebug                    \
          -ltapp                      \
          -lmnfuncs                   \
          -lmeteo.global              \
          -lmappi.global              \
          -lmeteo.textproto           \
          -lmeteo.map                 \
          -lmeteo.map.view            \
          -lmappi.proto               \
          -lsatlayer                  \
          -lsatelliteimage            \
          -lmappi.fileservice.client  \
          -lmappi.pos                 \
          -lmappi.mask                \
          -lmappi.schedule

LIBS +=           -L$(BUILD_DIR)/lib/plugins/mappi.app.widgets \
                  -L$(BUILD_DIR)/lib/plugins/mappi.map.viewer

                  
PROTOPATH += $(SRC)/meteo/commons/proto
include( $$(SRC)/protobuf.pri )
include( $$(SRC)/include.pro )
