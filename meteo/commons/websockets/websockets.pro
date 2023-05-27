TEMPLATE = lib
TARGET = meteowebsockets

_UNAME_ = $$(_UNAME_)
isEmpty(_UNAME_):_UNAME_ = $$lower( $$system( uname -a ) )
contains( _UNAME_, ".*astra.*" ) {

QT += core-private network

PUB_HEADERS     =       qwebsockets_global.h            \
                        qwebsocket.h                    \
                        qwebsocketserver.h              \
                        qwebsocketprotocol.h            \
                        qwebsocketcorsauthenticator.h   \
                        qmaskgenerator.h

PRIVATE_HEADERS =       qwebsocket_p.h                  \
                        qwebsocketserver_p.h            \
                        qwebsocketprotocol_p.h          \
                        qwebsockethandshakerequest_p.h  \
                        qwebsockethandshakeresponse_p.h \
                        qwebsocketdataprocessor_p.h     \
                        qwebsocketcorsauthenticator_p.h \
                        qwebsocketframe_p.h             \
                        qdefaultmaskgenerator_p.h       

SOURCES =               qwebsocket.cpp                  \
                        qwebsocket_p.cpp                \
                        qwebsocketserver.cpp            \
                        qwebsocketserver_p.cpp          \
                        qwebsocketprotocol.cpp          \
                        qwebsockethandshakerequest.cpp  \
                        qwebsockethandshakeresponse.cpp \
                        qwebsocketdataprocessor.cpp     \
                        qwebsocketcorsauthenticator.cpp \
                        qwebsocketframe.cpp             \
                        qmaskgenerator.cpp              \
                        qdefaultmaskgenerator_p.cpp     
contains(QT_CONFIG, ssl) | contains(QT_CONFIG, openssl) | contains(QT_CONFIG, openssl-linked) {
    SOURCES += 		qsslserver.cpp
    PRIVATE_HEADERS +=  qsslserver_p.h
}
}


include( $$(SRC)/include.pro )
