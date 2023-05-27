TEMPLATE = app test
TARGET   = test

SOURCES =     main.cpp          \


LIBS +=       -ltapp            \
              -ltdebug          \


include( $$(SRC)/include.pro )
