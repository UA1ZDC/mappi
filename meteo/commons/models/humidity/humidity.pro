TEMPLATE = lib
TARGET   = meteo.humidity

HEADERS             = funcs.h                     \

SOURCES             = funcs.cpp                   \

FORTRAN_SOURCES     = vlazh.for                   \

include( $$(SRC)/include.pro )
include( $$(SRC)/fortran.pri )

DATA_PATH = share/meteo/models/humidity

DEFINES +=  HUMIDITY_DATA_PATH=\\\"$$DATA_PATH/\\\"

HUM_FILES = data/*

humdata.files = $$HUM_FILES
humdata.path  = $$(BUILD_DIR)/$$DATA_PATH
!win32 { humdata.extra = chmod a+r $$HUM_FILES }
INSTALLS += humdata
