TEMPLATE            = lib
TARGET              = tcontainer

HEADERS             = thash.h               \
                      ts_list.h             \
                      ringvector.h          \
                      blockingqueue.h       \

SOURCES             = thash.cpp             \
                      ringvector.cpp        \
                      blockingqueue.cpp

include( $$(SRC)/include.pro )
