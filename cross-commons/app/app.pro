TEMPLATE = lib
TARGET =   tapp

QTV = $$(QT_VERSION)

contains (QTV, QT4) {
PUB_HEADERS =   paths.h                 \
                tsigslot.h              \
                targ.h                  \
                terror.h                \
                helpformatter.h         \
                waitloop.h              \
                options.h

HEADERS =       $$PUB_HEADERS           \

SOURCES =       targ.cpp                \
                paths.cpp               \
                terror.cpp              \
                waitloop.cpp            \              
                options.cpp

#LIBS +=         -ltincludes
}
contains (QTV, QT5) {
PUB_HEADERS =   paths.h                 \
                tsigslot.h              \
                targ.h                  \
                terror.h                \
                options.cpp             \
                helpformatter.h         \
                waitloop.h              \
                options.h

SOURCES =       targ.cpp                \
                paths.cpp               \
                terror.cpp              \
                waitloop.cpp            \              
                options.cpp

#LIBS +=         -ltincludes
}
contains (QTV, QT3) {
PUB_HEADERS =   paths.h                 \
                terror.h

SOURCES =       paths.cpp               \
                terror.cpp

#LIBS +=         -ltincludes-qt3
}

include( $$(SRC)/include.pro )

