TEMPLATE = app test
TARGET   = test

HEADERS =     test.h

SOURCES =     main.cpp

LIBS +=       -ltapp                  \
              -ltdebug                \

LIBS +=       -lmeteo.planner         \

include( $(SRC)/include.pro )

