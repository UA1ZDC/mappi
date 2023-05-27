TEMPLATE = app
TARGET = meteo.obanal.tfp
CONFIG += debug

SOURCES = temp_func.cpp \
          obanaltfp.cpp \
          main.cpp
   

HEADERS = obanaltfp.h \
          temp_func.h

LIBS    =       -lprotobuf              \
                -ltapp                  \
                -ltdebug                \
                -ltrpc                  \
                -ltsingleton            \
                -lmeteodata             \
                -lmeteo.nosql           \
                -lmeteo.dbi             \
                -lmnprocread            \
                -lmeteo.punchrules      \
                -lobanal                \
                -lmeteo.font            \
                -lmeteo.obanal          \
                -lmeteo.proto           \
                -lmeteo.global          \
                -lmeteo.settings        \
                -lmeteo.textproto       \
                -lzond                  \
                -lmnmathtools


ISOLINE_FILES = 5*.conf
isoline.files = $$ISOLINE_FILES
isoline.path  = $$(BUILD_DIR)/etc/meteo/isoline.conf.d
!win32 {isoline.extra = chmod a+r $$ISOLINE_FILES}
INSTALLS += isoline

CRON_FILES  = *.cron.conf
crn.token = env
crn.files = $$CRON_FILES
crn.path  = $$(BUILD_DIR)/etc/meteo/cron.d
!win32 { app.extra = chmod a+r $$CRON_FILES }
INSTALLS += crn

INCLUDEPATH +=  $(SRC)/meteo/commons/rpc

QMAKE_CXXFLAGS += -Wno-error=unused-function

include( $$(SRC)/include.pro )
