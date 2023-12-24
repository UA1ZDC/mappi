TARGET   = thematiccalc
TEMPLATE = lib
QT       -= core

DESTDIR  = $(BUILD_DIR)/lib

SOURCES  += thematiccalc.cpp

HEADERS  += thematiccalc.h


LIBS +=   -ltapp                      \
          -ltdebug                    \


include( $$(SRC)/include.pro )
QMAKE_CXXFLAGS += -Wno-error=class-memaccess
