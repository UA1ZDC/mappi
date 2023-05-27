TEMPLATE = app
TARGET   = meteo.createanimation

QT += widgets

HEADERS = 
      

SOURCES = main.cpp


LIBS += -ltapp			\
        -lmeteo.global		\
        -lmeteo.settings	\
        -lsql.proto             \
        -lmeteo.dbi             \
        -lmeteo.sql             \
        -lmeteo.nosql           \
        -lmeteo.map             \
        -ltdebug		\
        -lmeteo.punchrules      \
	-lprotobuf		\
	-ltcustomui		\
	-lmeteo.mainwindow	\
	-lmeteo.map.dataexchange\
	-lmeteo.map.view


exists ( /usr/bin/Magick++-config ) {
  LIBS += `Magick++-config --libs`
  QMAKE_CXXFLAGS += `Magick++-config --cxxflags --cppflags` -fopenmp -DMAGICKCORE_QUANTUM_DEPTH=16
  QMAKE_CXXFLAGS += -std=c++11 -Wno-error=undef
}
else {
  LIBS += -lMagick++-6.Q16  -lMagickCore-6.Q16
  QMAKE_CXXFLAGS += `pkg-config --cflags ImageMagick`
  QMAKE_CXXFLAGS += -std=c++11
}


include( $$(SRC)/include.pro )

QMAKE_CFLAGS += -Wno-error=unused-parameter
QMAKE_CXXFLAGS += -Wno-unused-parameter
