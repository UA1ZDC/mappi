TARGET    = mappi.cleaner
TEMPLATE  = app
QT       += script

LOG_MODULE_NAME = dbtask

HEADERS =       archivetask.h           \
                removetask.h            \
                checktask.h             \
                dbtask.h                \
                cleartabletask.h        \
                cleartabledirtask.h     \
                funcs.h                 \
                appstatus.h             \


SOURCES =       main.cpp                \
                archivetask.cpp         \
                removetask.cpp          \
                checktask.cpp           \
                dbtask.cpp              \
                cleartabletask.cpp      \
                cleartabledirtask.cpp   \
                appstatus.cpp           \

LIBS +=         -lprotobuf              \

LIBS +=         -ltapp                  \
                -ltdebug                \
                -ltrpc                  \
                -lnspgbase              \

LIBS +=         -lmeteo.global          \
                -lmeteo.planner         \
                -lmeteo.textproto       \
                -lmeteo.proto           \

LIBS +=         -lmappi.global           \
                -lmappi.proto            \
                -lmappi.settings         \

include( $$(SRC)/include.pro )
