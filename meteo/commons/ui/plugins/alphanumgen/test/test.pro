TEMPLATE = app test
TARGET   = test

PUB_HEADERS =

PRIVATE_HEADERS = ../alphanumgen.h \
                  ../kn01.h \
                  ../kn04.h \
                  ../kn03.h \
                  ../ancgen.pb.h 

HEADERS = $$PUB_HEADERS $$PRIVATE_HEADERS

SOURCES = ../alphanumgen.cpp \
          ../kn01.cpp \
          ../kn03.cpp \
          ../kn04.cpp \
          ../ancgen.pb.cc \
          test.cpp


FORMS = ../kn01.ui  ../kn03.ui  ../kn04.ui  ../selectweather.ui


LIBS +=                 -ltdebug                \
                        -ltapp                  \
                        -lnovost.global \
                        -lnovost.settings \
                        -lprotobuf \
                        -lmeteo.textproto \
                        -lmeteo.proto     \
                        -ltrpc \
                        -lmeteodata    \
                        -ltalphanum    \
                        -ltmeteocontrol \
                        -lmnmathtools \


PROTOPATH +=  $$(BUILD_DIR)/include /usr/include

include( $$(SRC)/include.pro )
LIBS += -L$(BUILD_DIR)/lib/plugins/meteo.app.widgets
