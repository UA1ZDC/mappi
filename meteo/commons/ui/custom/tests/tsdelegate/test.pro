TEMPLATE = app test
TARGET   = test

SOURCES = main.cpp
FORMS   = form.ui

LIBS +=       -ltapp                   \
              -ltdebug                 \
              -lcustom.tableview      \
              -ltcustomui              \

include( $$(SRC)/include.pro )

