TEMPLATE = lib
TARGET = tsingleton

LOG_MODULE_NAME="Singleton"

PUB_HEADERS =   tsingleton.h

HEADERS     =   $$PUB_HEADERS $$PRIVATE_HEADERS

SOURCES     =   tsingleton.cpp

LIBS       +=


PRO_PATH = $$system( pwd )
include( $$(SRC)/include.pro )


