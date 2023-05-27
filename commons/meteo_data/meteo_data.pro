TEMPLATE = lib
TARGET   = meteodata

LOG_MODULE_NAME = codecontrol

SOURCES = meteo_data.cpp  \
          tmeteodescr.cpp \
          complexmeteo.cpp

PUB_HEADERS = meteo_data.h  \
              tmeteodescr.h \
              complexmeteo.h

HEADERS = $$PUB_HEADERS $$PRIVATE_HEADERS

LIBS += -ltsingleton        \
        -ltapp              \
        -ltdebug            \
        -lmeteo.geobasis    \
        -lmnmathtools       \

PRO_PATH = $$system( cd )
include( $$(SRC)/include.pro )

descr.files = descriptors.xml
descr.path  = $$(BUILD_DIR)/etc/meteo/
INSTALLS += descr
