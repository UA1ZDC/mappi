TEMPLATE  = app
TARGET    = mappi.alphanum

QT       += widgets

LOG_MODULE_NAME = codecontrol

SOURCES = main.cpp  

LIBS += -ltalphanum

include( $$(SRC)/include.pro )

DECODERS = ./decoders/*.xml
decoders.files = $$DECODERS
decoders.path = $$(BUILD_DIR)/var/meteo/decoders
INSTALLS += decoders
