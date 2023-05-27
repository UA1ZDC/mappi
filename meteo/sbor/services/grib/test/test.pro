TARGET = test 
TEMPLATE = test
QT += xml

SOURCES = test.cpp
HEADERS = 

LIBS += -L$(BUILD_DIR)/lib      -ltapp            \
                                -ltdebug          \
                                -lmeteo.sql        \
                                -lmeteodata       \
                                -ltgribdecode     \
#                                -ltgribsave       \
                                -ltgribiface      \                               
                                -lmeteo.decodeserv \
                                -lprotobuf     \
                                -lmeteo.proto  \
                                -lmeteo.global \
                                -lprimarydb    \
                                -lmeteo.settings \
                                -lmeteo.dbi



include( $$(SRC)/include.pro )
