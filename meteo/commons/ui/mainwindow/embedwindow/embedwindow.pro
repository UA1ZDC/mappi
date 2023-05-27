TEMPLATE = lib
TARGET   = meteo.embedwindow

QT +=  widgets

LOG_MODULE_NAME = vizdoc

HEADERS =               appwatcher.h                    \
                        preloader.h                     \

SOURCES =               appwatcher.cpp                  \
                        preloader.cpp                   \

FORMS =                 preloader.ui                    \

HEADERS       += appcontainer_windows.h
SOURCES       += appcontainer_windows.cpp

LIBS                += -ltdebug                         \

include( $$(SRC)/include.pro )
