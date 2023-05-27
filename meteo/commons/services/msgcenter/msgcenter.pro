TEMPLATE = lib
TARGET   = meteo.msgcenter

QT      += network

LOG_MODULE_NAME = recgmi

HEADERS =     msgcenter.h               \
              appmain.h                 \
              router.h                  \
              methodbag.h               \
              notconfirmedchecker.h


SOURCES =     msgcenter.cpp             \
              appmain.cpp               \
              router.cpp                \
              methodbag.cpp             \
              notconfirmedchecker.cpp

LIBS +=       -lprotobuf

LIBS +=       -ltapp                    \
              -ltdebug                  \
              -ltrpc                    \
              -lmeteo.nosql             \
              -lmeteo.msgparser         \
              -lmeteo.proto             \
              -lmeteo.global            \
              -lmeteo.msgdata           \
              -lmeteo.geobasis          \
              -ltsingleton              \
              -ltrpc                    \
              -lmeteo.settings          \
              -lmeteo.map.view          \
              -lmeteo.map.dataexchange  \
              -ltalphanum               \
              -lmeteodata


include( $$(SRC)/include.pro )
