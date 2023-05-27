TEMPLATE = app
TARGET   = mappi.pretreatment.server

HEADERS = 
SOURCES = main.cpp

LIBS    = -ltapp            \
          -ltdebug          \
          -lnspgbase
          -ltrpc            \
          -lmappi.po.handler \
          -lmnmathtools     \
          -lmeteo.geobasis  \
          -lmnsatellite     \
          -lmappi.global    \
          -lmappi.settings  \
          -lmeteo.global    \
          -ltsingleton      \
          -lprotobuf        \
          -lmeteo.textproto \
          -lmeteo.proto     \
          -lmappi.proto     \


include( $$(SRC)/include.pro )
