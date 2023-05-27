TEMPLATE = lib
TARGET   = meteo.proto

QT -= gui core xml

PUB_PROTOS =  msgsettings.proto         \
              appconf.proto             \
              fullgmoconvert.proto

PROTOS =      $$PUB_PROTOS              \
              alphanum.proto            \
              bufr.proto                \
              authconf.proto            \
              climat.proto              \
              nodeservice.proto         \
              decoders.proto            \
              exportmap.proto           \
              field.proto               \
              forecast.proto            \
              locsettings.proto         \
              forecastwidget.proto      \
              meteo.proto               \
              map_city.proto            \
              map_document.proto        \
              map_isoline.proto         \
              map_ornament.proto        \
              map_radar.proto           \
              document_service.proto    \
              meteomenu.proto           \
              meteoproperty.proto       \
              meteotablo.proto          \
              msgcenter.proto           \
              msgparser.proto           \
              puanson.proto             \
              services.proto            \
              sigwx.proto               \
              sprinf.proto              \
              state.proto               \
              surface.proto             \
              surface_mongo.proto       \
              surface_service.proto     \
              sxfcodes.proto            \
              sxfloader.proto           \
              tgribformat.proto         \
              tgrid.proto               \
              tproduct.proto            \
              vcut_settings.proto       \
              ptkppstream.proto         \
              cleaner.proto             \
              usersettings.proto        \
              storm.proto               \
              gmoconvert.proto          \
              msgstream.proto           \
              cron.proto                \
              weather.proto             \
              meteogram.proto           \
              documentviewer.proto      \
              rpc_v1.proto              \
              spo_options.proto         \
              process.proto		\
              intersettings.proto       \
              web.proto                 \
              taskservice.proto         \
              pogodainput.proto         \
              obanal.proto              \
              customviewer.proto        \
              esimo.proto
              

LIBS +=         -lprotobuf

PROTOPATH +=    $(SRC)/meteo/commons/grib
PROTOPATH +=    $(SRC)/meteo/commons/rpc

include( $$(SRC)/include.pro )
include( $$(SRC)/protobuf.pri )
