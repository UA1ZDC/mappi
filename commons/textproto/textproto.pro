TEMPLATE = lib
TARGET   = meteo.textproto
LOG_MODULE_NAME = prototext
PUB_HEADERS             = tprototext.h      \
                          pbtools.h

HEADERS                 = $$PUB_HEADERS     \
                          pbcache.h

SOURCES                 = tprototext.cpp    \
                          pbtools.cpp       \
                          pbcache.cpp

LIBS                   += -lprotobuf        \
                          -ltdebug

PYMODULES               = prototext.py

include( $$(SRC)/include.pro )

