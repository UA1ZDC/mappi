TEMPLATE    =   lib
TARGET      =   mappi.receiver

HEADERS     =   ctrl.h \
                recvstream.h

SOURCES     =   ctrl.cpp \
                recvstream.cpp


LIBS       +=   -ltdebug \
                -ltapp  \
                -lmnsatellite \
                -lmappi.global \
                -lmappi.settings \
                -lmappi.po.handler

OTHER_FILES = *.py

PROTOPATH += $(SRC)/meteo/commons/proto
include( $$(SRC)/protobuf.pri )

include( $$(SRC)/include.pro )

py.files = $$OTHER_FILES
py.path  = $$(BUILD_DIR)/bin
!win32 {etc.extra = chmod a+x $$OTHER_FILES}
INSTALLS += py
