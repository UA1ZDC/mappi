TEMPLATE = lib
TARGET   = mappi.etc

RESOURCES   = mappi.qrc

SQL_QUERIES  = sql/*.sql

THEM_FILES = thematics/*.conf

OTHER_FILES = reception.conf          \
              satinstr.conf           \
              satpretr.conf           \
              thematics.conf          \
              thematics_calc.conf     \
              palette.conf            \
              composites.conf          \
              ftploader.conf

METEO_ETC_FILES =  map.settings.conf            

#              loc.settings.conf       \
#              log.settings.conf       \
#              cleaner.conf            \

#CLEANER_FILES = cleaner.d/*.conf



etc.token = env propfile
etc.files = $$OTHER_FILES
etc.path  = $$(BUILD_DIR)/etc/mappi
!win32 {etc.extra = chmod a+r $$OTHER_FILES}
INSTALLS += etc

#cleaner.token = env propfile
#cleaner.files = $$CLEANER_FILES
#cleaner.path = $$(BUILD_DIR)/etc/mappi/cleaner.d
#!win32 {etc.extra = chmod a+r $$CLEANER_FILES}
#INSTALLS += cleaner

weather.files = weather.txt
weather.path = $$(BUILD_DIR)/var/mappi/
INSTALLS += weather

meteoetc.token = env propfile
meteoetc.files = $$METEO_ETC_FILES
meteoetc.path  = $$(BUILD_DIR)/etc/meteo
unix:meteoetc.extra = chmod a+r $$METEO_ETC_FILES
INSTALLS += meteoetc

sql.files = $$SQL_QUERIES
sql.path  = $$(BUILD_DIR)//share/meteo/sql.d
!win32 {sql.extra = chmod a+r $$SQL_QUERIES}
INSTALLS += sql


them.token = env
them.files = $$THEM_FILES
them.path  = $$(BUILD_DIR)/etc/mappi/thematics
!win32 { app.extra = chmod a+r $$THEM_FILES  }
INSTALLS += them


MAINWINDOW_CONF_FILES = mainwindow.menu/*.conf
mainwindow.token = env
mainwindow.files = $$MAINWINDOW_CONF_FILES
mainwindow.path  = $$(BUILD_DIR)/etc/meteo/mainwindow.menu
INSTALLS += mainwindow
OTHER_FILES += $$MAINWINDOW_CONF_FILES

               
MAPWINDOW_CONF_FILES  = document.menu.conf/*.conf
mapwindow.token = env
mapwindow.files = $$MAPWINDOW_CONF_FILES
mapwindow.path  = $$(BUILD_DIR)/etc/meteo/document.menu.conf
!win32 { app.extra = chmod a+r $$MAPWINDOW_CONF_FILES  }
INSTALLS += mapwindow

               
APP_FILES           = app.conf.d/*.conf
app.token = env
app.files = $$APP_FILES
app.path  = $$(BUILD_DIR)/etc/meteo/app.conf.d
!win32 { app.extra = chmod a+r $$APP_FILES }
INSTALLS += app
OTHER_FILES += APP_FILES

DB_CONF_FILES       = db.conf.d/*.conf
db.token = env
db.files = $$DB_CONF_FILES
db.path  = $$(BUILD_DIR)/etc/meteo/db.conf.d
!win32 { app.extra = chmod a+r $$DB_CONF_FILES }
INSTALLS += db

SERVICES_CONF_FILES = services.conf.d/*.conf
srv.token = env
srv.files = $$SERVICES_CONF_FILES
srv.path  = $$(BUILD_DIR)/etc/meteo/services.conf.d
!win32 { app.extra = chmod a+r $$SERVICES_CONF_FILES }
INSTALLS += srv

include( $$(SRC)/include.pro )
