QT += core gui
greaterThan(QT_MAJOR_VERSION, 4) : QT += widgets

TARGET = exprguid
TEMPLATE = lib

DESTDIR = $(BUILD_DIR)/lib

include(exprdraw/exprdraw.pri)
include(exprmake/exprmake.pri)

include( $$(SRC)/include.pro )

# http://www.delphikingdom.com/asp/viewitem.asp?catalogid=718
