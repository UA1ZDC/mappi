TEMPLATE    =   lib
TARGET      =   tgrib

PRIVATE_HHEADERS =                

PUB_HEADERS =   tgribdefinition.h 
HEADERS     =   $$PUB_HEADERS $$PRIVATE_HEADERS

SOURCES     =   

LIBS       +=  -lprotobuf -lmeteo.proto


include( $$(SRC)/include.pro )
#PROTOS = *.proto
include( $$(SRC)/protobuf.pri)
#protobuf_decl.path


