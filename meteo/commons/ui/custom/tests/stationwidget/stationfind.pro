TEMPLATE = test
TARGET   = test
QT +=  widgets  

SOURCES =       main.cpp              \

LIBS +=         -ltcustomui           \

LIBS +=         -ltapp                \
                -ltdebug              \

LIBS +=         -lmeteo.geobasis      \
                -lmeteo.proto         \
                -lmeteo.global   \
                -lmeteo.settings

LIBS +=         -lproduct.settings \
                -lmeteo.map.view \
                -lmeteo.map \
                -lmeteo.map.dataexchange \
                -lmeteo.mainwindow
                
include( $$(SRC)/include.pro )

DESTDIR = .
