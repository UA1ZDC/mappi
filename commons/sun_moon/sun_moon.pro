TEMPLATE = lib
TAGET    = sun_moon

LOG_MODULE_NAME = prepare

PRIVATE_HEADERS =

PUB_HEADERS =  src/func_sun_moon.h

HEADERS = $$PUB_HEADERS $$PRIVATE_HEADERS

SOURCES = src/func_sun_moon.cpp

LIBS += -lmnmathtools

PRO_PATH = system( pwd )
include( $$(SRC)/include.pro )
