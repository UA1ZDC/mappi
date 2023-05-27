TEMPLATE = lib
TARGET   = meteo.msis

HEADERS         = nrlmsise-00.h               \
                  funcs.h                     \

SOURCES         = nrlmsise-00.c               \
                  nrlmsise-00_data.c          \
                  funcs.cpp                   \

DEFINES        += MSIS_INLINE                 \

include( $$(SRC)/include.pro )
