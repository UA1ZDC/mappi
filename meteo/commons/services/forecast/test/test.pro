TEMPLATE = app
TARGET   = testforecastservice

SOURCES =       test.cpp

LIBS +=       -ltrpc            \
              -ltapp            \
              -ltdebug          \
              -lprotobuf        \
              -lobanal          \
              -lmeteo.geobasis  \
              -lmeteo.proto     \
              -lmeteo.global    \
              -lmeteo.forecast  \
              -lzond            \
              -lmeteo.settings  \
              -lmeteo.global  \
              -lmnprocread  \
              -lmeteo.dbi

PRO_PATH = $$system( pwd )
include( $$(SRC)/include.pro )
