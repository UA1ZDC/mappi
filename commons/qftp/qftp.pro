#load(qt_build_config)

TEMPLATE = lib
TARGET = qftp
#CONFIG += static
#CONFIG -= shared
QT = core network

#MODULE_PRI = ../../modules/qt_ftp.pri
#MODULE = ftp

#load(qt_module)

# Input
HEADERS += qftp.h qurlinfo.h
SOURCES += qftp.cpp qurlinfo.cpp

include( $$(SRC)/include.pro )
