TEMPLATE = lib
TARGET   = meteo.astrodata

INCLUDEPATH += ./ ./extlib

QT += core gui printsupport


HEADERS = \
    extlib/AstroOps.h \
    extlib/DateOps.h \
    extlib/Lunar.h \
    extlib/LunarTerms.h \
    extlib/MathOps.h \
    extlib/PlanetData.h \
    extlib/Pluto.h \
    extlib/PlutoTerms.h \
    extlib/RiseSet.h \
    extlib/TimeOps.h \
    extlib/Vsop.h \
    defines.h \
    exception.h \
    astrodata.h \
    astrodatainfo.h \
    astrodatafile.h \
    astrodatamodel.h \
    observerpoint.h \
    astrodatadocument.h \
    header.h

SOURCES = \
    extlib/astroOps.cpp \
    extlib/DateOps.cpp \
    extlib/DateOpsIntl.cpp \
    extlib/Lunar.cpp \
    extlib/MathOps.cpp \
    extlib/PlanetData.cpp \
    extlib/Pluto.cpp \
    extlib/RiseSet.cpp \
    extlib/TimeOps.cpp \
    extlib/Vsop.cpp \
    extlib/VsopData.cpp \
    astrodata.cpp \
    astrodatainfo.cpp \
    astrodatafile.cpp \
    astrodatamodel.cpp \
    observerpoint.cpp \
    astrodatadocument.cpp \
    header.cpp
#    main.cpp \

LIBS += \
    -lmeteo.etc \
    -lmeteo.global \
    -ltdebug \
    -lmeteo.geobasis  \
    -lmeteo.sql          \
    -ltapp

include( $$(SRC)/include.pro )


