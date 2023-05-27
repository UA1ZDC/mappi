TEMPLATE              = lib
TARGET                = meteo.customviewer.service

LOG_MODULE_NAME = inter

LIBS                 += -ltdebug         \
                        -ltapp           \
                        -lmeteo.etc      \
                        -lmeteo.proto    \
                        -lprotobuf       \
                        -lmeteo.settings \
                        -lmnprocread     \
                        -lmeteo.dbi      \
                        -ltrpc           \
                        -lmeteo.global   \
                        -ltmeteocontrol  \
                        -lmeteo.msgparser\
                        -lmeteo.geobasis \
                        -lmeteodata\
                        -ltgribdecode \
                        -ltgribiface \
                        -ltalphanum \
                        -ltbufr\
                        -ltcustomui \
                        -lmeteo.map \
                        -lmeteo.map.view \
                        -lmeteo.map.dataexchange \
                        -lmeteo.mainwindow \
                        -lmeteo.decodeviewer \
                        -lmeteo.ui.condition \
                        -lmeteo.textproto \
                        -lmeteo.nosql

include( $$(SRC)/include.pro )

SOURCES += \
    appmain.cpp \
    customviewerservice.cpp \
    customviewerservicehandler.cpp \
    counter.cpp \
    loader.cpp \
    postwork.cpp

HEADERS += appmain.h \
    customviewerservice.h \
    customviewerservicehandler.h \
    counter.h \
    loader.h \
    postwork.h

RESOURCES += \
    configs.qrc
