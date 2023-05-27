TEMPLATE = lib
QT += script  widgets

TARGET = meteo.forecastwidgetcore
LOG_MODULE_NAME = forecastwidget

HEADERS  += forecastwidget.h \
            forecastresultwidget.h \
            forecastviewer.h       \
            forecastaccuracy.h     \
            forecastdatamodel.h    \            
            forecastaccuracydatamodel.h


SOURCES +=  forecastwidget.cpp \            
            forecastresultwidget.cpp \
            forecastviewer.cpp       \
            forecastaccuracy.cpp     \
            forecastdatamodel.cpp    \
            forecastaccuracydatamodel.cpp


FORMS += forecastwidget.ui \
         forecastresultwidget.ui


LIBS += -lprotobuf                      \

LIBS += -ltapp                          \
        -ltdebug                        \
        -ltrpc                          \
        -lmeteo.proto                   \
        -lmeteo.map                     \
        -lmeteo.map.view                \
        -lmeteo.global                  \
        -lmeteo.geobasis                \
        -lmeteodata                     \
        -lmeteo.mainwindow              \
        -lobanal                        \
        -ltcustomui                     \
        -lmnmathtools                   \
        -lmeteo.forecast                \
        -lmeteo.proto                   \
        -lmeteo.etc                     \
        -lmeteo.customviewer

include( $$(SRC)/include.pro )

#DESTDIR = $(BUILD_DIR)/lib/
