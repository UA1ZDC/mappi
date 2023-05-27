TEMPLATE = lib
TARGET   = alphanumgen.plugin

QT += widgets

PUB_HEADERS =

PRIVATE_HEADERS = alphanumgen.h \
                  kn01.h \
                  kn03.h \
                  kn04.h \
                  alphanumgenplugin.h
                  
SOURCES = alphanumgen.cpp \
          kn01.cpp \
          kn03.cpp \
          kn04.cpp \
          alphanumgenplugin.cpp

FORMS = kn01.ui \ 
        kn04.ui \
        kn03.ui \
        selectweather.ui

PROTOS =  ancgen.proto

LIBS +=  -lprotobuf  \
         -lmeteo.textproto \
         -lmeteo.global \
         -lmeteo.proto  \
         -lmeteodata    \
         -ltalphanum    \
         -ltmeteocontrol \
         -lmnmathtools
         
LIBS +=                 -ltapp                          \
                        -ltdebug                        \
                        -ltrpc                          \
                        -lmeteo.proto                   \
                        -lmeteo.map                     \
                        -lmeteo.map.view                \
                        -lmeteo.global                  \
                        -lmeteo.geobasis                \
                        -lmeteodata                     \
                        -lmeteo.sql                      \
                        -lmeteo.mainwindow              \
                        -lobanal                        \
                        -ltcustomui                     \
                        -lmnmathtools                   \
                        -lmeteo.etc


templ.files = ./templates/*.templ  ./templates/*.html
templ.path = $$(BUILD_DIR)/share/meteo/anc-templates

INSTALLS += templ

include( $$(SRC)/protobuf.pri )
include( $$(SRC)/include.pro )

DESTDIR = $(BUILD_DIR)/lib/plugins/meteo.app.widgets
