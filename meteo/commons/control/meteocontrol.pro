TEMPLATE = lib
TARGET   = tmeteocontrol

PUB_HEADERS +=   tcontrolmethods.h   \
                 tmeteocontrol.h     \
                 tmethodstempl.h     \
                 tlimitsurf.h        \
                 tlimitair.h         \
                 tconsistency_surf.h \
                 tconsistency_air.h  \
                 tconsistency_wind.h \
                 tconsistency_sign.h \
                 tconsistency_time.h

HEADERS = $$PUB_HEADERS $$PRIVATE_HEADERS

SOURCES +=  tcontrolmethods.cpp   \
            tmeteocontrol.cpp     \
            tlimitsurf.cpp        \
            tlimitair.cpp         \
            tconsistency_surf.cpp \
            tconsistency_air.cpp  \
            tconsistency_wind.cpp \
            tconsistency_sign.cpp \
            tconsistency_time.cpp


LIBS += -ltdebug \
        -ltapp   \
        -ltsingleton  \
        -lmnmathtools \
        -lmeteodata

include( $$(SRC)/include.pro )

settings.files = xml/*.xml
settings.path  = $$(BUILD_DIR)/etc/meteo/control
INSTALLS += settings
