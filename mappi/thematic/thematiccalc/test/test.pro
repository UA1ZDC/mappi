TARGET   = test

TEMPLATE = test console

QT       -= core

CONFIG += console
CONFIG += no_keywords #AsmJit use emit keyword
CONFIG -= app_bundle


SOURCES += main.cpp

#INCLUDEPATH += $(HOME)/dev/asmjit/src/asmjit

LIBS +=   -ltapp                      \
          -ltdebug                    \
          -lmeteo.geobasis            \
          -lthematiccalc              \
          -lasmjit

include( $$(SRC)/include.pro )
QMAKE_CXXFLAGS += -Wno-error=class-memaccess
