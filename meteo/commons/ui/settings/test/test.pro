TEMPLATE = app test
TARGET   = test
QT += widgets
SOURCES =       main.cpp

LIBS +=       -ltapp                      \
              -ltdebug                    \
              -lnovost.settings            \
              -lmeteo.uisettings	  \
	      -lnovost.proto		  \
	      -ltrpc			  \
              -lprotobuf


include( $$(SRC)/include.pro )

