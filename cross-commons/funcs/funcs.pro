TEMPLATE = lib
TARGET = crossfuncs

PRIVATE_HEADERS =

PUB_HEADERS =             \
  mn_errdefs.h            \
  mn_funcs.h

SOURCES =                 \
#  mn_errdefs.cpp          \
  mn_funcs.cpp

#QT += widgets


include( $$(SRC)/include.pro )
