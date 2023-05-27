TEMPLATE = app
TARGET   = mappi.ftploader

LOG_MODULE_NAME = recgmi

QT += network core

HEADERS =           rawloader.h

SOURCES =           main.cpp                      \
                    rawloader.cpp

LIBS +=             -lprotobuf                    \

LIBS +=             -ltapp                        \
                    -ltdebug                      \
                    -ltrpc                        \
                    -lnspgbase                    \
                    -lqftp                        \

LIBS +=             -lmeteo.proto                 \
                    -lmeteo.global                \

LIBS +=             -lwrf.global                  \
                    -lwrf.proto                   \
                    -lwrf.settings                \
                    -lwrf.ftploader.core          \

include( $$(SRC)/include.pro )


