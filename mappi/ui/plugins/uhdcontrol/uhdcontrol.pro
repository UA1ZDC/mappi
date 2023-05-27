QT       += core gui widgets

TEMPLATE = lib
TARGET   = mappi.uhdcontrolplugin

LIBS += -lprotobuf              \
        -lmeteo.geom            \
        -ltapp                  \
        -ltdebug                \
        -lmeteo.proto           \
        -lmeteo.map             \
        -lmeteo.map.view        \
        -lmeteo.geobasis        \
#        -lmeteo.graph           \
        -lmeteo.map.spectrcoordaction  \
        -lmeteo.map.spectrvalueaction  \
        -lboost_system          \
        -lmnmathtools           \
        -lfftw3                 \
        -luhd                   \
        -lmappi.etc

include( $$(SRC)/include.pro )

SOURCES += uhdcontrol.cpp   \
           layerspectr.cpp  \
           ramkaspectr.cpp  \
           gridspectr.cpp   \
           axisspectr.cpp   \
           lineitem.cpp     \
           labelitem.cpp    \
           filterkalman.cpp \
           funcs.cpp        \
           uhdcontrolplugin.cpp \
           uhdwindow.cpp

HEADERS  += uhdcontrol.h   \
            layerspectr.h  \
            ramkaspectr.h  \
            gridspectr.h   \
            axisspectr.h   \
            lineitem.h     \
            labelitem.h    \
            filterkalman.h \
            funcs.h        \
            uhdcontrolplugin.h \
            uhdwindow.h

FORMS    += uhdcontrol.ui

DESTDIR = $(BUILD_DIR)/lib/plugins/mappi.app.widgets
