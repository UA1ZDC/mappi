TEMPLATE = app test
TARGET   = test

SOURCES =   main.cpp                \


LIBS +=         -L$(BUILD_DIR)/lib/plugins/meteo.app.widgets

LIBS +=         -lprotobuf              \

LIBS +=         -lmnmathtools           \
                -ltapp                  \
                -ltdebug                \
                -ltrpc                  \
                -lmeteodata             \
                -ltcustomui             \

LIBS +=         -lmeteo.global          \
                -lnovost.global         \
                -lnovost.settings       \
                -lmeteo.geobasis        \
                -lmeteo.map             \
                -lmeteo.map.view        \
                -lmeteo.proto           \
                -lmeteo.mainwindow      \
                -lmeteo.graph           \
                -lmeteo.meteogram.plugin \

include( $$(SRC)/include.pro )
