TEMPLATE = lib
TARGET   = mnmathtools

LOG_MODULE_NAME = applied

PRIVATE_HEADERS =

PUB_HEADERS =   mathematical_constants.h  \
                physical_constants.h      \
                mnmath.h                  \
                tgeometrymath.h           \
                atmosphysics.h            \
                radiophysics.h            \
                fft.h                     \
                complex.h

HEADERS = $$PUB_HEADERS $$PRIVATE_HEADERS

SOURCES =       mnmath.cpp              \
                tgeometrymath.cpp       \
                atmosphysics.cpp        \
                radiophysics.cpp        \ 
                fft.cpp                   \
                complex.cpp

PRO_PATH = $$system( pwd )
include( $$(SRC)/include.pro )
