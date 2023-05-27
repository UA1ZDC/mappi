TEMPLATE = test
TARGET   = test

SOURCES = test.cpp

LIBS +=  -ltapp  \
         -ltgribparser \
         -ltgrib \
         -ltgribdecode \
         -ltdebug \
         -lprotobuf \
#         -ltgribsave \
         -ltgribiface \
#         -lobanal \
         -lmeteo.global \
         -lmeteo.proto  \
         -lmeteo.settings \
         -lprotobuf \
         -ltrpc \
         -lmeteo.nosql

         
PRO_PATH = $$system( pwd )
include( $$(SRC)/include.pro )

