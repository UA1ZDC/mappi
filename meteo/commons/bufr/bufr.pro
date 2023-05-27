TEMPLATE = lib
TARGET   = tbufr

LOG_MODULE_NAME = codecontrol

PRIVATE_HEADERS = tbufrlist.h 


PUB_HEADERS = tbufr.h     \
              tbufrmeta.h \
              tbufrtransform.h \
              appmain.h


HEADERS = $$PUB_HEADERS $$PRIVATE_HEADERS

SOURCES = tbufr.cpp      \
          tbufrlist.cpp  \
          tbufrmeta.cpp  \
          tbufrtransform.cpp \
          appmain.cpp


LIBS +=   -lmeteodata         \
          -lprotobuf          \
          -ltdebug            \
          -ltapp              \
          -lmeteo.proto       \
          -lmeteo.global      \
          -ltmeteocontrol     \
          -lmeteo.decodeserv  \
#          -lcrossfuncs        \
          -lmeteo.geobasis    \
          -lmnprocread        \
          -lmeteo.sql          \
          -lprimarydb         \
          -lservicestate      \
          -ltsingleton


include( $$(SRC)/include.pro )

bufrgr.files = bufrtransform.xml
bufrgr.path = $$(BUILD_DIR)/etc/meteo/

tables.files = tables/*.txt
tables.path = $$(BUILD_DIR)/var/meteo/bufrtables

INSTALLS += bufrgr tables
