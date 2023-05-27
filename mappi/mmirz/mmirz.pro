TEMPLATE = app
CONFIG += debug
TARGET   = mmirz

QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

HEADERS = mmirz.h choosefile.h mmirz_res.h mmirz_graph.h widget.h qcustomplot.h

SOURCES = mmirz.cpp \
          choosefile.cpp \
          main.cpp  \
          mmirz_res.cpp  \
          mmirz_graph.cpp \
          widget.cpp    \
          qcustomplot.cpp


INCLUDEPATH += $(BUILD_DIR)/include

FORMS = mmirz.ui mmirz_graph.ui mmirz_res.ui choosefile.ui
LIBS +=  -L$(BUILD_DIR)/lib \
       -ltdebug \
 -lmnmathtools \
-lspschedule \
-lmnsatellite \
-lspcoordsys \
-lmeteo.geobasis  \
      -ltapp   \




#contains( ENV_QTV, QT4 ){
#  DEFINES += LOG_MODULE_NAME=\\\"$$mmirz\\\"
#}
#contains( ENV_QTV, QT3 ){
#  DEFINES += LOG_MODULE_NAME=\"$$mmirz\"
#}

include( $$(SRC)/include.pro )

#The following line was inserted by qt3to4
#QT +=  qt3support
