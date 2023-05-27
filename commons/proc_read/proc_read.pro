TEMPLATE = lib
TARGET   = mnprocread

QT += network

PUB_HEADERS     = checkps.h                       \
                  daemoncontrol.h                 \

!win32:SOURCES += checkps.cpp                     \
                  daemoncontrol_linux.cpp         \

HEADERS =         $$PUB_HEADERS

win32:SOURCES +=  daemoncontrol_win.cpp           \

LIBS +=           -ltapp                          \
                  -ltdebug                        \
#                  -lcrossfuncs                    \

include( $$(SRC)/include.pro )

SOURCES += \
    daemoncontrol.cpp
