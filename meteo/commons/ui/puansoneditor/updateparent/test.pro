TEMPLATE = test
TARGET   = test

SOURCES =     test.cpp

LIBS +=       -ltdebug                  \
              -ltapp                    \
              -lprotobuf                \
              -lmeteodata               \
              -lmeteo.sql

LIBS +=       -lmeteo.proto             \
              -lmeteo.global            \
              -lmeteo.mainwindow

LIBS +=       -lmeteo.map               \

LIBS +=       -lmaslo.global            \
              -lmaslo.settings		\
	      -lbank.global		\
	      -lbank.etc		\
	      -lbank.settings		

include( $$(SRC)/include.pro )
