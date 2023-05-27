TEMPLATE = lib
TARGET   = fieldwrap

SOURCES =      tfieldwrap.cpp

PRIVATE_HEADERS =

PUB_HEADERS =

HEADERS = $$PUB_HEADERS $$PRIVATE_HEADERS

LIBS += -lobanal

include( $$(SRC)/include.pro )

LIBS += $$L_BOOST_PYTHON
