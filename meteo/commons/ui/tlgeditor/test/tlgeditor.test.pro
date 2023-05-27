TEMPLATE  = test
TARGET    = test
LOG_MODULE_NAME = vizdoc
QT += widgets
SOURCES =   main.cpp

QT += script

LIBS +=     -ltapp                              \
            -ltdebug                            \

LIBS +=     -lmeteo.tlgeditor                  \
            -lmeteo.global                     \
            -lmeteo.settings                   \
            -ltgribiface

include( $$(SRC)/include.pro )


