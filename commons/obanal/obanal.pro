TEMPLATE = lib
TARGET   = obanal

SOURCES =       interpolorder.cpp       \
                tfield.cpp              \
                mngisdata.cpp           \
                func_obanal.cpp         \
                haosmain.cpp                \
                linalgebra.cpp          \
                obanal_struct.cpp       \
                tisolinedata.cpp        \
                tfieldparams.cpp        \
                rbspline.cpp            \
                tfieldmongo.cpp

PRIVATE_HEADERS =

PUB_HEADERS =   interpolorder.h         \
                haosmain.h                  \
                tfield.h                \
                mngisdata.h             \
                func_obanal.h           \
                linalgebra.h            \
                obanal_struct.h         \
                tisolinedata.h          \
                tfieldparams.h          \
                rbspline.h              \
                tfieldmongo.h


HEADERS = $$PUB_HEADERS $$PRIVATE_HEADERS

LIBS += -lprotobuf          \

LIBS += -lmnmathtools       \
        -lmeteo.geobasis    \
        -lcommons.proto     \
        -ltapp              \
        -ltdebug            \
        -lmeteo.textproto   \

include( $$(SRC)/include.pro )

