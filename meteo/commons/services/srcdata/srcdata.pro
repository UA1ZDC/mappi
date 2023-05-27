TEMPLATE = lib
TARGET   = meteo.srcdata
CONFIG += thread

LOG_MODULE_NAME = dbconn


SOURCES =               tdata_service.cpp         \
                        meteodata_service.cpp     \
                        tzond_data_service.cpp    \
                        tsurface_data_service.cpp \
                        tradar_data_service.cpp   \
                        tgrib_data_service.cpp    \
                        ocean_data_service.cpp    \
                        geophys_data_service.cpp  \
                        base_data_service.cpp     \
                        dataloader.cpp \
    appmain.cpp

PUB_HEADERS =           tdata_service.h           \
                        meteodata_service.h       \
                        tzond_data_service.h      \
                        tradar_data_service.h     \
                        tgrib_data_service.h      \
                        tsurface_data_service.h   \
                        ocean_data_service.h      \
                        geophys_data_service.h    \
                        base_data_service.h       \
                        dataloader.h              \
                        appmain.h


HEADERS =       types.h

LIBS +=         -lprotobuf              \
                -lmeteo.proto           \
                -lmeteo.global          \
                -lmeteo.textproto       \
                -lmeteo.geobasis        \
                -lmeteo.msgcenter       \
                -lmeteo.msgdata         \
                -lmeteo.etc             \
                -lmnmathtools           \
                -lmeteo.sql              \
                -ltdebug                \
                -ltapp                  \
                -ltrpc                  \
                -lmeteodata             \
                -ltgribiface            \
                -lzond                  \
                -lprimarydb             \
                -ltalphanum             \
                -lmeteo.settings        \
                -lmnprocread            \
                -ltcustomui             \
                -lmeteo.mainwindow      \
                -lmeteo.map.view        \
                -lmeteo.map.dataexchange\
		-lmeteo.sprinf

include( $$(SRC)/include.pro )
