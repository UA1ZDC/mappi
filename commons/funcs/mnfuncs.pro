TEMPLATE = lib
TARGET = mnfuncs

QT += network widgets 

PUB_HEADERS =  fs.h \
               tcolorgrad.h            \
               mnio.h                  \
               tlgtextobject.h       
                        
SOURCES =      fs.cpp \
               selfaddress.cpp         \
               tcolorgrad.cpp          \
               mnio.cpp                \
               tlgtextobject.cpp               
                        
#win32:SOURCES += win/tpartitioninfo.cpp
#unix:SOURCES  += linux/tpartitioninfo.cpp


HEADERS += selfaddress.h

LIBS += -lmnmathtools   \
        -ltincludes     \
        -ltapp          \
        -ltdebug        

include( $$(SRC)/include.pro )

defined( T_OS_ASTRA_LINUX, var ) {
LIBS +=       -lparsec-mac              \
              -lparsec-cap              \
              -lparsec-base
}
