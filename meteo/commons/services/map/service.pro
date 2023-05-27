TEMPLATE = lib
TARGET   = meteo.map.service
QT += concurrent widgets

LOG_MODULE_NAME = docmaker

SOURCES =       mapservice.cpp          \
                methodbag.cpp           \
                appstatus.cpp           \
                tfaxdb.cpp              \
                appmain.cpp

HEADERS =       mapservice.h            \
                methodbag.h             \
                appstatus.h             \
                appmain.h               \
                tfaxdb.h

LIBS +=           -ltapp                    \
                  -ltdebug                  \
                  -lprotobuf                \
                  -lmeteo.geobasis          \
                  -lmeteo.proto             \
                  -lmeteo.map               \
                  -lmeteo.map.oldgeo        \
                  -lmeteo.map.dataexchange  \
                  -lmeteo.tileimage         \
                  -ltrpc                    \
                  -ltsingleton              \
                  -lmeteodata               \
                  -lobanal                  \
                  -lmeteo.global            \
                  -lzond                    \
                  -lmnprocread              \
                  -lmeteo.fieldata          \
                  -lmeteo.settings          \
                  -lmeteo.mainwindow        \
                  -lmeteo.map.view          \
                  -lmeteo.map.ptkpp         \
                  -ltcustomui

include( $$(SRC)/include.pro )

QMAKE_CXXFLAGS += -Wno-pmf-conversions
