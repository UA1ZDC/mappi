TEMPLATE = lib
TARGET   = meteo.map

QT += widgets
QT -= xml network concurrent

LOG_MODULE_NAME = map
PUB_HEADERS =           aerodocument.h          \
                        formaldoc.h             \
                        document.h              \
                        incut.h                 \
                        legend.h                \
                        ornament.h              \
                        layer.h                 \
                        layeriso.h              \
                        layermrl.h              \
                        layergrid.h             \
                        layerpunch.h            \
                        layerfigure.h           \
                        layeritems.h            \
                        layersigwx.h            \
                        object.h                \
                        geopolygon.h            \
                        isoline.h               \
                        geogroup.h              \
                        geotext.h               \
                        geopixmap.h             \
                        geogradient.h           \
                        geomrl.h                \
                        puanson.h               \
                        label.h                 \
                        freezedlabel.h          \
                        weatherfigure.h         \
                        wind.h                  \
                        windpath.h              \
                        flow.h                  \
                        flowpath.h              \
                        cloudwx.h               \
                        ramka.h                 \
                        weather.h               \
                        event.h                 \
                        geoaxis.h               \
                        loader.h                \
                        commongrid.h            \
                        commonstation.h         \
                        layermenu.h             \
                        ptkppformat.h           \
                        customevent.h           \
                        map.h                   \
                        vprofiledoc.h           \
                        scale.h                 \
                        layervprofile.h         \
                        vprofile.h              \
                        graph/axisgraph.h       \
                        graph/funcs.h           \
                        graph/gridgraph.h       \
                        graph/layergraph.h      \
                        graph/ramkagraph.h      \
                        profile/layerprofile.h  \
                        profile/axisprofile.h   \
                        axis/axissimple.h       \
                        axis/ramkaaxis.h        \
                        axis/axispuanson.h      \
                        axis/layeraxis.h        \
                        vbuilder.h              \
                        objectmimedata.h        \
                        puansonairport.h        \
                        layerairport.h          \
                        layerborder.h


                        
PRIVATE_HEADERS =       arrows/arrow.h          \
                        arrows/simple.h



SOURCES =               document.cpp            \
                        incut.cpp               \
                        legend.cpp              \
                        ornament.cpp            \
                        layer.cpp               \
                        layeriso.cpp            \
                        layermrl.cpp            \
                        layergrid.cpp           \
                        layerpunch.cpp          \
                        layerfigure.cpp         \
                        layeritems.cpp          \
                        layersigwx.cpp          \
                        object.cpp              \
                        geopolygon.cpp          \
                        isoline.cpp             \
                        geogroup.cpp            \
                        geotext.cpp             \
                        geopixmap.cpp           \
                        geogradient.cpp         \
                        geomrl.cpp              \
                        puanson.cpp             \
                        label.cpp               \
                        freezedlabel.cpp        \
                        weatherfigure.cpp       \
                        wind.cpp                \
                        windpath.cpp            \
                        flow.cpp                \
                        flowpath.cpp            \
                        cloudwx.cpp             \
                        ramka.cpp               \
                        weather.cpp             \
                        event.cpp               \
                        geoaxis.cpp             \
                        loader.cpp              \
                        commongrid.cpp          \
                        commonstation.cpp       \
                        layermenu.cpp           \
                        ptkppformat.cpp         \
                        arrows/simple.cpp       \
                        map.cpp                 \
                        customevent.cpp         \
                        vprofiledoc.cpp         \
                        scale.cpp               \
                        layervprofile.cpp       \
                        vprofile.cpp            \
                        aerodocument.cpp        \
                        formaldoc.cpp           \
                        graph/axisgraph.cpp     \
                        graph/funcs.cpp         \
                        graph/gridgraph.cpp     \
                        graph/layergraph.cpp    \
                        graph/ramkagraph.cpp    \
                        profile/layerprofile.cpp \
                        profile/axisprofile.cpp \
                        axis/axispuanson.cpp    \
                        axis/axissimple.cpp     \
                        axis/ramkaaxis.cpp      \
                        axis/layeraxis.cpp      \
                        vbuilder.cpp            \
                        objectmimedata.cpp      \
                        puansonairport.cpp      \
                        layerairport.cpp        \
                        layerborder.cpp


LIBS +=                 -lobanal                \
                        -lzond                  \
                        -lprotobuf              \
                        -lmeteodata             \
                        -lmeteo.global          \
                        -lmeteo.proto           \
                        -lmeteo.textproto       \
                        -lmeteo.geom            \
                        -lmeteo.radar           \
                        -lmeteo.font            \
                        -lmeteo.sql              \
                        -ltdebug                \
                        -lmnmathtools           \
                        -ltcontainer            \
                        -lmnfuncs               \
                        -lmeteo.geobasis        \
                        -ltapp                  \
                        -ltrpc                  \
                        -lcrossfuncs            \
                        -ltsingleton            \
                        -lmeteo.tileimage



INCLUDEPATH += arrows

include( $$(SRC)/include.pro )

QMAKE_CXXFLAGS -= -Wall
QMAKE_CFLAGS -= -Wall
