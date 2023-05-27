TEMPLATE = app
TARGET   = mappi.app.manager

LOG_MODULE_NAME = contrdiag

SOURCES =               main.cpp

#appmanagerdaemon.cpp

HEADERS =
#appmanagerdaemon.h

LIBS    =       -lmeteo.app.manager


#LIBS +=         -lprotobuf              \
#                -lqtservice

#LIBS +=         -ltapp                  \
#                -ltdebug                \
#                -ltrpc                  \
#                -lmnprocread

#LIBS +=         -lmeteo.app.manager     \
#                -lmeteo.proto           \
#                -lmeteo.global

#LIBS +=         -lmappi.global           \
#                -lmappi.settings

include( $$(SRC)/include.pro )
