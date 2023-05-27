TEMPLATE    =   lib
TARGET      =   mappi.po.instruments

PRIVATE_HHEADERS =

QMAKE_CXXFLAGS += -Wno-error=write-strings                

PUB_HEADERS =   avhrr3.h \
                msu.h    \
                msumr.h  \
                virr.h   \
                mtvza.h  \
                mhs.h    \
                amsua1.h \
                amsua2.h \
                msugs.h  \
                hrpt_calibr.h \
                hrpt_const.h \
                avhrr3cadu.h \
                viirs.h \
                modis.h


HEADERS     =   $$PUB_HEADERS $$PRIVATE_HEADERS

SOURCES     =   avhrr3.cpp   \   
                msu.cpp      \
                msumr.cpp    \
                virr.cpp     \
                mtvza.cpp    \
                mhs.cpp      \
                amsua1.cpp   \
                amsua2.cpp   \
                msugs.cpp    \
                hrpt_calibr.cpp \
                hrpt_const.cpp \
                avhrr3cadu.cpp \
                viirs.cpp    \
                modis.cpp

                

LIBS       +=   -ltdebug \
                -lmappi.po.formats \
                -lmappi.po.frame \
                -laec \
                -lgdal
                


include( $$(SRC)/include.pro )

factors.files = factors/*.txt
factors.path  = $$(BUILD_DIR)/share/mappi/factors
INSTALLS += factors
