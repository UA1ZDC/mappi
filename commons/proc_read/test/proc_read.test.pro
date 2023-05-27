TEMPLATE = test
TARGET   = test

SOURCES +=    main.cpp

LIBS +=       -ltapp                  \
              -ltdebug                \
              -lmnprocread            \

include( $$(SRC)/include.pro )
