TEMPLATE = lib
TARGET = meteo.forecast

LOG_MODULE_NAME = forecast

QT += uitools script  scripttools

PUB_HEADERS =             tforecast.h \
                          tdataprocessor.h \
                          tdataprovider.h \
                          tfdelegate.h \
                          tfmodel.h \
                          tfitem.h \
                          tforecastlist.h \
                          tffielditem.h \
                          tfdatetimeitem.h


SOURCES +=  tforecast.cpp \
                          tdataprovider.cpp \
                          tfdelegate.cpp \
                          tfmodel.cpp \
                          tfitem.cpp    \
                          tforecastlist.cpp \
                          tffielditem.cpp \
                          tfcomboitem.cpp \
                          tdataprocessor.cpp \
                          tfdatetimeitem.cpp


FR =          ui/forecast_simple.ui         \
              ui/forecast1result.ui         \
              ui/forecast_simple_new.ui     \
              ui/forecast1.ui               \
              ui/mainforecastform.ui        \
              ui/mainforecastform2.ui       \
              ui/test.ui

HEADERS =   tfcomboitem.h \
    tfdatetimeitem.h


LIBS += -lmeteodata             \
        -lmeteo.etc             \
        -lmeteo.proto           \
        -lmeteo.global          \
        -lzond                  \
        -lprotobuf              \
        -lmeteo.astrodata

LIBS +=                 -ltapp                          \
                        -ltdebug                        \
                        -ltrpc                          \
                        -lmeteo.proto                   \
                        -lmeteo.global                  \
                        -lmeteo.geobasis                \
                        -lmeteodata                     \
                        -lmeteo.sql                      \
                        -lobanal                        \
                        -lmeteo.textproto               \
#                        -ltcustomui                     \
                        -lmnmathtools

include( $$(SRC)/include.pro )

SCRIPTS_FILES += \
    ./scripts/forecastmain.js \
    ./scripts/forecastMagnusMoisture.js \
    ./scripts/forecastTitovVNGO.js \
    ./scripts/forecastTitovTmax.js \
    ./scripts/forecastVayting.js \
    ./scripts/forecastFaust.js \
    ./scripts/forecastVaytingFaust.js \
    ./scripts/forecastReshetov1v2.js \
    ./scripts/forecastPinus400un.js \
    ./scripts/forecastVeljtishev.js \
   ./scripts/forecastOrlovaLivn.js \
    ./scripts/forecastMDV500.js \
    ./scripts/forecastSlavin.js \
    ./scripts/forecastSosin.js \
   ./scripts/forecastVeterSvAtm.js \
   # ./scripts/forecastIlyina.js    \
    ./scripts/MagnusMoisture.js \
    ./scripts/TitovVNGO.js      \
    ./scripts/TitovTmax.js   \
    ./scripts/forecastMDV3km.js \
    ./scripts/MDV3km.js         \
    ./scripts/TitovTmax.js      \
    ./scripts/Vayting.js        \
    ./scripts/Faust.js    \
    ./scripts/VaytingFaust.js    \
    ./scripts/Reshetov1v2.js    \
    ./scripts/MDV500.js    \
    ./scripts/Slavin.js    \
    ./scripts/Sosin.js   \
    ./scripts/VeterSvAtm.js   \
    ./scripts/CloudAmount.js  \
   ./scripts/forecastCloudAmount.js  \
    ./scripts/RubinshtCloud.js  \
    ./scripts/forecastRubinshtCloud.js  \
    ./scripts/forecastSparyshCloud.js  \
    ./scripts/SparyshCloud.js   \
    ./scripts/Pinus400un.js    \
    ./scripts/Veljtishev.js    \
    ./scripts/OrlovaLivn.js    \
  #  ./scripts/Ilyina.js    \
    ./scripts/forecastNGOwithAdv.js \
    ./scripts/NGOwithAdv.js   \
    ./scripts/OneHourVNGO.js  \
    ./scripts/forecastOneHourVNGO.js \
    ./scripts/forecastSurfaceWind.js \
    ./scripts/SurfaceWind.js  \
    ./scripts/forecastLowHightGO.js  \
    ./scripts/LowHightGO.js  \
    ./scripts/forecastCloudEvo.js  \
    ./scripts/CloudEvo.js  \
    ./scripts/forecastMDVprecip.js  \
    ./scripts/MDVprecip.js  \
    ./scripts/forecastNDV.js \
    ./scripts/NDV.js  \
    ./scripts/forecastComplTempZ.js  \
    ./scripts/ComplTempZ.js  \
    ./scripts/forecastVertDvij.js  \
    ./scripts/VertDvij.js  \
    ./scripts/forecastVlajSvobAtm.js  \
    ./scripts/VlajSvobAtm.js  \
    ./scripts/forecastTemperSvobAtm.js  \
    ./scripts/TemperSvobAtm.js  \
    ./scripts/forecastAdvTum.js  \
    ./scripts/AdvTum.js  \
    ./scripts/forecastVnutGololed.js \
    ./scripts/VnutGololed.js   \
    ./scripts/forecastGololedica.js  \
    ./scripts/Gololedica.js  \
    ./scripts/forecastFrGololed.js  \
    ./scripts/FrGololed.js   \
    ./scripts/forecastObledHPol.js  \
    ./scripts/ObledHPol.js    \
    ./scripts/Obled850700.js  \
    ./scripts/forecastObled850700.js  \
    ./scripts/forecastPinus400up.js  \
    ./scripts/Pinus400up.js  \
    ./scripts/forecastPinusPrizem.js  \
    ./scripts/PinusPrizem.js  \
    ./scripts/forecastOblOrlova.js  \
    ./scripts/OblOrlova.js  \
    ./scripts/forecastBerlyand.js  \
    ./scripts/Berlyand.js  \
    ./scripts/forecastZverev.js  \
    ./scripts/Zverev.js  \
    ./scripts/Ilina.js  \
    ./scripts/forecastIlina.js  \
    ./scripts/forecastMetel.js  \
    ./scripts/Metel.js  \
    ./scripts/forecastTurbul.js  \
    ./scripts/Turbul.js  \
    ./scripts/forecastPoryv.js  \
    ./scripts/Poryv.js  \
    ./scripts/Pochv.js  \
    ./scripts/forecastPochv.js  \
    ./scripts/forecastVolna.js  \
    ./scripts/Volna.js

scripts.files = $$SCRIPTS_FILES
scripts.path = $$(BUILD_DIR)/var/meteo/forecast/scripts
scripts.extra = chmod a+r $$SCRIPTS_FILES

ui.files = $$FR
ui.path = $$(BUILD_DIR)/var/meteo/forecast/ui
ui.extra = chmod a+r $$FR

INSTALLS += scripts ui

OTHER_FILES += \
    scripts/CloudAmount.js \
    scripts/forecastCloudAmount.js
