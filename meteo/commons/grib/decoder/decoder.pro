TEMPLATE    =   lib
TARGET      =   tgribdecode

LOG_MODULE_NAME = codecontrol

PRIVATE_HHEADERS = tgrib1_decode.h \
                   tgrib2_decode.h \
                   tdecodebase.h  \
                   appmain.h

PUB_HEADERS =   tgribdecode.h 
HEADERS     =   $$PUB_HEADERS $$PRIVATE_HEADERS

SOURCES     =   tgrib2_decode.cpp \
                tgrib1_decode.cpp \
                tgribdecode.cpp \
                tdecodebase.cpp \
                appmain.cpp

LIBS       +=   -ltapp        \
                -ltdebug      \
                -lmeteo.sql    \
                -lprotobuf    \
                -ltgribparser \
                -ltgrib       \
                -lmnmathtools \
                -lmeteo.decodeserv  \
#                -lcrossfuncs        \
                -lmeteo.global      \
                -lmeteo.proto       \
                -lmnprocread        \
                -lmeteo.settings    \
                -lservicestate      \
                -ltsingleton        \
                -lprimarydb


                
include( $$(SRC)/include.pro )

xml.files = xml/*
xml.path  = $$(BUILD_DIR)/etc/meteo/
INSTALLS += xml

