TEMPLATE = lib
TARGET   = meteo.planner

LOG_MODULE_NAME = planner

HEADERS =         planner.h         \
                  timesheet.h       \
                  configloader.h    \

SOURCES =         planner.cpp       \
                  timesheet.cpp     \
                  configloader.cpp  \

LIBS +=           -ltdebug          \

LIBS +=           -lmeteo.textproto \

include( $(SRC)/include.pro )
