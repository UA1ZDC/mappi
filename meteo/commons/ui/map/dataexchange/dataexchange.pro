TEMPLATE = lib
TARGET   = meteo.map.dataexchange

QT += widgets

HEADERS  = exchange_algorithms.h    \
           passportbasis.h          \
           sxfexchange.h            \
           sxfstructures.h          \
           sxfobjects.h             \
           sxftomap.h               \
           rscobjects.h             \
           metasxf.h
           

SOURCES  = exchange_algorithms.cpp  \
           sxfexchange.cpp          \
           sxfstructures.cpp        \
           sxfobjects.cpp           \
           sxftomap.cpp             \
           rscobjects.cpp           \
           metasxf.cpp
           

LIBS    += -lmeteo.geobasis        \
           -lmeteo.map             \
           -ltsingleton            \
           -lprotobuf              \
           -ltapp                  \
           -ltdebug                \
           -lmeteo.proto           \
           -lmeteodata             \
           -lmnmathtools           \
           -lmeteo.textproto       \

include( $$(SRC)/include.pro )
