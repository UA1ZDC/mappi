TEMPLATE = lib
TARGET   = meteo.aerodiag.core
QT += widgets
LOG_MODULE_NAME = aerodiag

SOURCES =       layer_aerodiag.cpp  \
                aerodiagdata.cpp \
                createaerodiagwidget.cpp \
                aeroindexes.cpp \
                aeroaction.cpp


HEADERS =       layer_aerodiag.h  \
                aerodiagdata.h \
                createaerodiagwidget.h \
                aeroindexes.h \
                aeroaction.h

FORMS = createaerodiagwidget.ui \
        aeroindexes.ui


RESOURCES =     vc.qrc

LIBS +=                 -lprotobuf              \
                        -ltdebug                \
                        -lmnmathtools           \
                        -ltrpc                  \
                        -lobanal                \
                        -lmeteodata             \
                        -lmeteo.geobasis        \
                        -lmeteo.proto           \
                        -lmeteo.global          \
                        -lmeteo.etc             \
                        -lmeteo.map             \
                        -lmeteo.map.view        \
                        -lzond                  \
                        -lmeteo.aerotable.core
                        
LIBS +=                 -ltapp                  \
                        -lmeteo.sql              \
                        -lmeteo.mainwindow      \
                        -lobanal                \
                        -ltcustomui             \
                        -lmnmathtools           \
                        -lmeteo.textproto       \
                        -lmeteo.map.dataexchange\
                        -lmeteo.verticalcut.core \
                        -lmeteo.stationlist

                        
include( $$(SRC)/include.pro )
