TEMPLATE = test
TARGET   = test
QT += widgets

SOURCES = test.cpp

LIBS +=                 -ltdebug                \
                        -ltapp                  \
                        -lmeteo.geobasis        \
                        -lmeteo.proto           \
#                        -lmap.settings          \
#                        -lnovost.etc            \
                        -lmeteo.puansoneditor.plugin

PROTOPATH +=  $$(BUILD_DIR)/include /usr/include

include( $$(SRC)/include.pro )
