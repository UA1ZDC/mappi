TEMPLATE = lib
TARGET   = tincludes

PRIVATE_HEADERS =

PUB_HEADERS =   tcommoninclude.h        \
                tcrossdefines.h

QTV = $$(QT_VERSION)

SOURCES     =   tcommoninclude.cpp

contains( QTV, QT4 ) {
PUB_HEADERS +=  tqt4include.h           \
#                tqt4mocinclude.h

SOURCES +=      tqt4include.cpp         \
#                tqt4mocinclude.cpp
} else {
contains( QTV, QT5 ) {
PUB_HEADERS +=  tqt4include.h           \
#                tqt4mocinclude.h

SOURCES +=      tqt4include.cpp         \
#                tqt4mocinclude.cpp
} else {
#PUB_HEADERS +=  tqt3include.h           \
#                tqt3mocinclude.h

#SOURCES +=      tqt3include.cpp         \
#                tqt3mocinclude.cpp
}
}

HEADERS = $$PUB_HEADERS $$PRIVATE_HEADERS

PRO_PATH = system( pwd )
include( $$(SRC)/include.pro )
