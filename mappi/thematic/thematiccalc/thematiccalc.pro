TARGET   = thematiccalc
TEMPLATE = lib
QT       -= core
CONFIG += no_keywords #AsmJit use emit keyword

DESTDIR  = $(BUILD_DIR)/lib

SOURCES  += thematiccalc.cpp \
    rpn.cpp

HEADERS  += thematiccalc.h \
    rpn.h


LIBS +=   -ltapp                      \
          -ltdebug                    \
          -lasmjit                    \


include( $$(SRC)/include.pro )
QMAKE_CXXFLAGS += -Wno-error=class-memaccess
