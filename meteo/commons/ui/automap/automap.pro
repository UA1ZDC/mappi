TEMPLATE = lib
TARGET = meteo.automap
QT += widgets printsupport

SOURCES = \
  maptoolwidget.cpp \
  automapmodel.cpp \
  joblist.cpp \
  automap.cpp \
  prerun.cpp

HEADERS = \
  maptoolwidget.h \
  automapmodel.h \
  joblist.h \
  automap.h \
  prerun.h

FORMS = \
  maptoolwidget.ui \
  automap.ui \
  prerun.ui

LIBS += \
  -ljobwidget \
  -lmnfuncs \
  -lprotobuf \
  -lmeteo.planner \
  -lmeteo.textproto \
  -lmeteo.etc \
  -lmeteo.global \
  -lmeteo.proto \
  -lmeteo.settings \
  -lmeteo.document.view \
  -lmeteo.map.client

include( $$(SRC)/include.pro )
