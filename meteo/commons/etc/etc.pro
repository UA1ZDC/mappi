TEMPLATE = aux
TARGET   = meteo.etc

PUNCH_FILES = punch/*.template
FORECAST_FILES = forecast/*.template

OBANAL_CONF = obanal/*.ini

WEATHER_MAPS = weather/*.map
PUNCH_MAPS = punch.map/*.map

             
CONF_FILES =  sxf/sxfcodes.conf     \
              sxf/sxfloader.conf    \
              application.menu.conf \
              location.conf         \
              meteotablo.conf       \
              ancmongo.conf         \
              vko.conf              \
              esimo.conf
              
SXF_RSC    = sxf/meteoznaki_v1.rsc \
             sxf/meteoznaki_v2.rsc \
             sxf/meteoznaki_v3.rsc \
             sxf/world_1M.rsc      \
             sxf/world_meteo.sxf

RADAR_FILES  = radar.conf.d/*.conf

PYCONF              = conf.py
pyconf.token = env
pyconf.files = $$PYCONF
pyconf.path  = $$(BUILD_DIR)/include.py
!win32 {pyconf.extra = chmod a+r $$PYCONF}
INSTALLS += pyconf
OTHER_FILES  += $$PYCONF

ORNAMENT_FILES = ornament.conf.d/*.conf

FORECAST_WIDGET  = forecastwidget/*.json 
FORECAST_CONF    = forecastwidget/*.ini

MAPWINDOW_CONF_FILES  = document.menu.conf/*.conf

MONGO_QUERIES  = mongo/*.json
MONGO_QUERIES += mongo/forecast/*.json
MONGO_QUERIES += mongo/save_meteodata/*.json
MONGO_QUERIES += mongo/find_bufr_descr/*.json
MONGO_QUERIES += mongo/grib/*.json
MONGO_QUERIES += mongo/customviewer/*.json
MONGO_QUERIES += mongo/gmi_condition/*.json
MONGO_QUERIES += mongo/typec_area/*.json
MONGO_QUERIES += mongo/sprinf/*.json
MONGO_QUERIES += mongo/common/*.json
MONGO_QUERIES += mongo/docservice/*.json
MONGO_QUERIES += mongo/fax/*.json
MONGO_QUERIES += mongo/images_satelite/*.json
MONGO_QUERIES += mongo/conditionmaker/*.json
MONGO_QUERIES += mongo/climat/*.json
MONGO_QUERIES += mongo/maslo.cleaner/*.json
MONGO_QUERIES += mongo/user_positions/*.json
MONGO_QUERIES += mongo/user_appointments/*.json
MONGO_QUERIES += mongo/user_settings/*.json
MONGO_QUERIES += mongo/messages/*.json
MONGO_QUERIES += mongo/journal/*.json
MONGO_QUERIES += mongo/inter.tasks/*.json
MONGO_QUERIES += mongo/user_departments/*.json

SQL_QUERIES  = sql/*.sql

MONGO_FUNCS  = mongo_funcs/*.js

OSAUTH_FILES =  os-auth.conf \
                ald-users.conf

OTHER_FILES         = $$CONF_FILES              \
                      $$MAPWINDOW_CONF_FILES    \
                      $$OSAUTH_FILES
                    

punch.files = $$PUNCH_FILES
punch.path  = $$(BUILD_DIR)/share/meteo/punch
!win32 {punch.extra = chmod a+r $$PUNCH_FILES}
INSTALLS += punch

forecast.files = $$FORECAST_FILES
forecast.path  = $$(BUILD_DIR)/share/meteo/forecast
!win32 {forecast.extra = chmod a+r $$FORECAST_FILES}
INSTALLS += forecast

weather.files = $$WEATHER_MAPS
weather.path  = $$(BUILD_DIR)/share/meteo/weather
!win32 {weather.extra = chmod a+r $$WEATHER_MAPS}
INSTALLS += weather

punchmap.files = $$PUNCH_MAPS
punchmap.path  = $$(BUILD_DIR)/share/meteo/punch.map
!win32 {punchmap.extra = chmod a+r $$PUNCH_MAPS}
INSTALLS += punchmap

conf.files = $$CONF_FILES
conf.path  = $$(BUILD_DIR)/etc/meteo/
!win32 {conf.extra = chmod a+r $$CONF_FILES}
INSTALLS += conf

sxfrsc.files = $$SXF_RSC
sxfrsc.path  = $$(BUILD_DIR)/share/meteo/
!win32 {sxfrsc.extra = chmod a+r $$SXF_RSC}
INSTALLS += sxfrsc

radar.files = $$RADAR_FILES
radar.path  = $$(BUILD_DIR)/etc/meteo/radar.conf.d
!win32 {radar.extra = chmod a+r $$RADAR_FILES}
INSTALLS += radar

ornament.files = $$ORNAMENT_FILES
ornament.path  = $$(BUILD_DIR)/share/meteo/ornament.conf.d
!win32 {ornament.extra = chmod a+r $$ORNAMENT_FILES}
INSTALLS += ornament

mongo.files = $$MONGO_QUERIES
mongo.path  = $$(BUILD_DIR)//share/meteo/mongo.d
!win32 {mongo.extra = chmod a+r $$MONGO_QUERIES}
INSTALLS += mongo

sql.files = $$SQL_QUERIES
sql.path  = $$(BUILD_DIR)//share/meteo/sql.d
!win32 {sql.extra = chmod a+r $$SQL_QUERIES}
INSTALLS += sql

mongofuncs.files = $$MONGO_FUNCS
mongofuncs.path  = $$(BUILD_DIR)//share/meteo/mongo.funcs
!win32 {mongofuncs.extra = chmod a+r $$MONGO_FUNCS}
INSTALLS += mongofuncs
   
forecastwidget.files = $$FORECAST_WIDGET
forecastwidget.path  = $$(BUILD_DIR)//share/meteo/forecastwidget
!win32 {forecastwidget.extra = chmod a+r $$FORECAST_WIDGET}
INSTALLS += forecastwidget

forecastconf.files = $$FORECAST_CONF
forecastconf.path  = $$(BUILD_DIR)//etc/meteo/
!win32 {forecastconf.extra = chmod a+r $$FORECAST_CONF}
INSTALLS += forecastconf

obanalconf.files = $$OBANAL_CONF
obanalconf.path  = $$(BUILD_DIR)//etc/meteo/
!win32 {obanalconf.extra = chmod a+r $$OBANAL_CONF}
INSTALLS += obanalconf

mapwindow.token = env
mapwindow.files = $$MAPWINDOW_CONF_FILES
mapwindow.path  = $$(BUILD_DIR)/etc/meteo/document.menu.conf
!win32 { app.extra = chmod a+r $$MAPWINDOW_CONF_FILES  }
INSTALLS += mapwindow



osauth.files = $$OSAUTH_FILES
osauth.path  = $$(BUILD_DIR)/etc/meteo
INSTALLS += osauth

include( $$(SRC)/include.pro )

DISTFILES += \
    mongo/get_undecoded_fax_tlg.json \
    sql/clear_nodes.sql \
    sql/count_all_recv_tlg.sql \
    sql/count_all_sended_tlg.sql \
    sql/find_blank.sql \
    sql/find_from_msg_viewer_distinct.sql \
    sql/find_station_by_station_and_type.sql \
    sql/get_all_req.sql \
    sql/get_blanks.sql \
    sql/get_meteo_data_by_coords.sql \
    sql/get_meteo_data.sql \
    sql/get_meteo_data_by_day.sql \
    sql/get_data_count.sql \
    sql/get_meteo_data_nearest_time.sql \
    sql/get_military_district.sql \
    sql/get_statistic_dest.sql \
    sql/get_statistic_src.sql \
    sql/get_undecoded_fax_tlg.sql \
    sql/meteodata_get_one_surface_data_2.sql \
    sql/meteodata_get_one_surface_data_1.sql \
    sql/get_one_meteo_data.sql \
    sql/get_available_stations.sql \
    sql/get_one_zond_data.sql \
    sql/get_available_zond.sql \
    sql/get_aero_date_list.sql \
    sql/get_near_date.sql \
    sql/get_stations.sql \
    sql/get_regions.sql \
    sql/get_many_ocean_data.sql \
    sql/get_available_ocean.sql \
    sql/get_available_synmobsea.sql \
    sql/get_ocean_by_station.sql \
    sql/remove_request.sql \
    sql/update_blanks.sql \
    sql/update_nodes.sql \
    sql/update_obanal.sql \
    sql/update_quality.sql \
    sql/update_quality_many.sql \
    sql/update_meteoparam.sql \
    sql/aggregate_obanal_get_available_data.sql \
    sql/load_field.sql \
    sql/get_available_centers.sql \
    sql/get_available_centers_forecast.sql \
    sql/get_last_date.sql \
    sql/get_field_dates.sql \
    sql/aggregate_get_fields_for_date.sql \
    sql/get_one_field.sql \
    sql/get_available_data_for_meteosummary.sql \
    sql/load_fields_for_puanson.sql \
    sql/get_field_descr_po_id.sql \
    sql/aggregate_get_fields_for_hour.sql \
    sql/insert_grib1.sql \
    sql/insert_grib2.sql \
    sql/load_grib_params.sql \
    sql/find_grib_not_analysed.sql \
    sql/grib_find.sql \
    sql/get_grib_data.sql \
    sql/update_request.sql \
    sql/update_station_history.sql \
    sql/get_data_cover.sql \
    sql/delete_regionfromsrc.sql \
    sql/get_data_count_max_min.sql
