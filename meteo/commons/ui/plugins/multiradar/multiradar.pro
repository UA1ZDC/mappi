TEMPLATE = lib
TARGET   = meteo.multiradar.plugin
LOG_MODULE_NAME = multiradar
FORMS =         multiradar.ui

PRIVATE_HEADERS =       multiradar.h            \
                        plugin.h

SOURCES =               multiradar.cpp          \
                        plugin.cpp

HEADERS = $$PUB_HEADERS  $$PRIVATE_HEADERS

exists ( /usr/bin/Magick++-config ) {
  LIBS += `Magick++-config --libs`
  QMAKE_CXXFLAGS += `Magick++-config --cxxflags --cppflags` -fopenmp -DMAGICKCORE_HDRI_ENABLE=0 -DMAGICKCORE_QUANTUM_DEPTH=16
  QMAKE_CXXFLAGS += -std=c++11 -Wno-error=undef
}
else {
  LIBS += -lMagick++-6.Q16
  QMAKE_CXXFLAGS += `pkg-config --cflags ImageMagick` -fopenmp -DMAGICKCORE_HDRI_ENABLE=0 -DMAGICKCORE_QUANTUM_DEPTH=16 -I/usr/include/x86_64-linux-gnu//ImageMagick-6 -I/usr/include/ImageMagick-6
  QMAKE_CXXFLAGS += -std=c++11
}

include( $$(SRC)/include.pro )

DESTDIR = $(BUILD_DIR)/lib/plugins/meteo.map.weather
