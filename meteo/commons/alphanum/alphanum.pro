TEMPLATE = lib
TARGET   = talphanum
QT += xml network

LOG_MODULE_NAME = codecontrol


PUB_HEADERS += tmeteoelement.h    \
               regexpdecode.h     \
               metadata.h         \
               wmodecoder.h       \
               tabledecoder.h     \
               stationmsg.h       \
               wmomsg.h           \
               cliwaremsg.h       \
               talphanum.h        \
               gphdecoder.h       \
               appmain.h          \
               xmllinedecode.h
               


HEADERS = $$PUB_HEADERS $$PRIVATE_HEADERS


SOURCES += tmeteoelement.cpp  \
           regexpdecode.cpp   \
           metadata.cpp       \
           wmodecoder.cpp     \
           tabledecoder.cpp   \
           stationmsg.cpp     \
           wmomsg.cpp         \
           cliwaremsg.cpp     \
           talphanum.cpp      \
           gphdecoder.cpp     \
           appmain.cpp        \
           xmllinedecode.cpp


LIBS += -ltsingleton      \
        -ltapp            \
        -ltdebug          \
        -lmeteodata       \
        -lprotobuf        \
        -ltrpc            \
        -lmeteo.sql       \
        -lmnmathtools     \
        -lmeteo.geobasis  \
        -lmeteo.textproto \
        -lmeteo.proto     \
        -lmeteo.global    \
        -ltmeteocontrol   \
        -lmeteo.nosql     \
        -lmeteo.settings  \
        -lmeteo.decodeserv\
        -lservicestate    \
        -lprimarydb       \
        -lmnprocread

include( $$(SRC)/include.pro )


element.files = andescriptors.xml clidescriptors.xml
element.path = $$(BUILD_DIR)/etc/meteo/decoders

codes.files = codeforms.xml
codes.path = $$(BUILD_DIR)/etc/meteo/decoders

INSTALLS += element codes
