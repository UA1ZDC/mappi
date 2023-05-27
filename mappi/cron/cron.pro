TEMPLATE = app
TARGET   = mappi.cron

#HEADERS =         runapp.h              \

SOURCES =         main.cpp

LIBS =          -lmeteo.cron            \
                -lmeteo.punchrules

                
#                  runapp.cpp

#LIBS +=           -lprotobuf            \

#LIBS +=           -ltapp                \
#                  -ltdebug              \

#LIBS +=           -lmeteo.planner       \
#                  -lmeteo.textproto     \

#LIBS +=           -lmappi.settings \
#                  -lmappi.global   \
#                  -lmappi.proto    \

include( $$(SRC)/include.pro )

