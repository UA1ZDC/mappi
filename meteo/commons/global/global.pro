TEMPLATE = lib
TARGET   = meteo.global

QT += network widgets dbus
QT += webchannel

LOG_MODULE_NAME = applied

PUB_HEADERS =   global.h                \
                log.h                   \
                websocketransport.h     \
                requestcard.h           \
                common.h                \
                appstatusthread.h       \
                binproto.h              \
                funcs.h                 \
                formaldocsaver.h        \
                dbnames.h               \
                dateformat.h            \
                gradientparams.h        \
                weatherloader.h         \
                radarparams.h           \
                loadcity.h

SOURCES =       global.cpp              \
                common.cpp              \
                appstatusthread.cpp     \
                binproto.cpp            \
                funcs.cpp               \
                formaldocsaver.cpp      \
                websocketransport.cpp   \
                dateformat.cpp          \
                gradientparams.cpp      \
                weatherloader.cpp       \
                radarparams.cpp         \
                loadcity.cpp


PYMODULES =     common.py/*.py

LIBS +=         -ltapp                  \
                -ltdebug                \
                -lprotobuf              \
                -ltrpc                  \
                -lmeteo.sql             \
                -lmeteodata             \
                -lmeteo.textproto       \
                -lcommons.proto         \
                -lmeteo.settings        \
                -lmeteo.proto           \
                -lmeteo.geobasis        \
                -lmeteo.punchrules      \
                -ltsingleton		\
		-lmeteo.dbi		\
		-lmeteo.sql		\
                -lmeteowebsockets       \
		-lmeteo.nosql

include( $$(SRC)/include.pro )

defined( T_OS_ASTRA_LINUX, var ) {
LIBS +=       -lparsec-mac              \
              -lparsec-cap              \
              -lparsec-base
  INCLUDEPATH += $(SRC)/meteo/commons/websockets
}
isEmpty(T_OS_ASTRA_LINUX) {
  QT += websockets
}



