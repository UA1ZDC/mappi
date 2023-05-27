TEMPLATE = app test

PUB_HEADERS =                       \
          oldzond/taero.h           \
          oldzond/func_common.h     \
          oldzond/func_obr_sond.h   \
          oldzond/funkciidiagnoza.h \
          oldzond/func_fillData.h   \
          oldzond/data_structures.h \
          oldzond/map_specification.h 


SOURCES =  \
          oldzond/taero.cpp           \
          oldzond/func_common.cpp     \
          oldzond/func_obr_sond.cpp   \
          oldzond/funkciidiagnoza.cpp \
          oldzond/func_fillData.cpp   \          
          main.cpp

PRIVATE_HEADERS = 

HEADERS = $$PUB_HEADERS $$PRIVATE_HEADERS


LIBS += \
#        -ltmeteoservice \
        -lmeteo.proto   \
        -lmeteo.srcdata \
        -lmeteo.global  \
        -lnovost.global \
        -lmnprocread   \
        -lprotobuf     \
        -ltapp         \
        -ltdebug       \
        -ltrpc         \        
        -ltgribsave    \        
        -lzond         \
        -lmeteodata

PRO_PATH = $$system( pwd )
include( $$(SRC)/include.pro )


#service.files = forecast.xml
#service.path  = $$(BUILD_DIR)/etc/meteo/service
#INSTALLS += service
