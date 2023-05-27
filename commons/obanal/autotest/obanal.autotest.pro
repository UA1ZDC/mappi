TEMPLATE    = autotest
TARGET      = test

HEADERS =     autotest.h                  \

SOURCES =     main.cpp                    \
              test_tfield.cpp

LIBS +=       -lcppunit                   \
              -lprotobuf                  \

LIBS +=       -ltapp                      \
              -ltdebug                    \
              -lobanal                    \
              -lmeteo.geobasis            \

include( $$(SRC)/include.pro )
