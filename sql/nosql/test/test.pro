TEMPLATE = test
TARGET   = test

SOURCES =           main.cpp

LIBS +=             -ltapp                \
                    -ltdebug              \
                    -lmeteo.sql            \
                    -lmeteo.geobasis      \
                    -lmnmathtools         \
                    -lprotobuf            \
                    -lmeteo.global        \
                    -lmeteo.proto         \
                    -lmeteo.settings      \
                    -ltrpc                \
                    -lmeteo.nosql

include( $$(SRC)/include.pro )

CONFIG += warn_off


