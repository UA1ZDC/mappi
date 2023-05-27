TEMPLATE = lib
TARGET   = spcoordsys

HEADERS	=	jullian.h	\
		projection_functions.h

SOURCES = 	jullian.cpp    	\
		projection_functions.cpp

LIBS += -lmnmathtools           \
        -lmnfuncs

include( $$(SRC)/include.pro )
