TEMPLATE = lib
TARGET   = mnsatellite

LOG_MODULE_NAME="spsatellite"

PRIVATE_HEADERS = \
  noradbase.h     \
  noradsgp4.h     \
  noradsdp4.h

PUB_HEADERS = satellite.h       \
              satellitebase.h   \
              satviewpoint.h    \
              tracktopopoint.hpp

HEADERS = $$PUB_HEADERS $$PRIVATE_HEADERS

SOURCES =               \
  satellite.cpp         \
  noradbase.cpp         \
  noradsgp4.cpp         \
  noradsdp4.cpp         \
  satellitebase.cpp     \
  satviewpoint.cpp

LIBS +=                 \
  -lmnmathtools         \
  -lspcoordsys          \
  -lmeteo.geobasis      \
#  -lmnfuncs             \
  -ltapp                \
  -ltdebug              \
  -lcrossfuncs

  
include( $$(SRC)/include.pro )
