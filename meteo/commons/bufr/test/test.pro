TEMPLATE = test
TARGET   = test

SOURCES += test.cpp

LIBS +=  -ltapp  \
#         -lmeteo.sql \
#         -lmeteo.nosql \
         -ltdebug \
         -ltbufr  \
         -lmeteodata \
         -lmeteo.global \
         -lprimarydb \
#         -lobanal    \
          -lmeteo.settings \
         -lmeteo.proto    \
         -ltrpc \
         -lmeteo.dbi
        

include( $$(SRC)/include.pro )


