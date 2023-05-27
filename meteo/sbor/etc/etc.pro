TEMPLATE = lib
TARGET = sbor.etc

CONF_FILES = \
  alphanum.conf \
  bufr.conf

DB_CONF_FILES       = db.conf.d/*.conf
SERVICES_CONF_FILES = services.conf.d/*.conf

APP_FILES   = app.conf.d/*.conf
CRON_FILES  = cron.d/*.conf
MENU_FILES  = mainwindow.menu/*.conf
ROUTE_FILES = route.conf.d/*.conf

OTHER_FILES = \
  $$CONF_FILES \
  $$APP_FILES \
  $$DB_CONF_FILES \
  $$SERVICES_CONF_FILES \
  $$MENU_FILES \
  $$ROUTE_FILES

etc.token = env
etc.files = $$CONF_FILES
etc.path  = $$(BUILD_DIR)/etc/meteo
!win32 { etc.extra = chmod a+r $$CONF_FILES }
INSTALLS += etc

app.token = env
app.files = $$APP_FILES
app.path  = $$(BUILD_DIR)/etc/meteo/app.conf.d
!win32 { app.extra = chmod a+r $$APP_FILES }
INSTALLS += app

db.token = env
db.files = $$DB_CONF_FILES
db.path  = $$(BUILD_DIR)/etc/meteo/db.conf.d
!win32 { db.extra = chmod a+r $$DB_CONF_FILES }
INSTALLS += db

menu.token = env
menu.files = $$MENU_FILES
menu.path  = $$(BUILD_DIR)/etc/meteo/mainwindow.menu
!win32 { menu.extra = chmod a+r $$MENU_FILES }
INSTALLS += menu

srv.token = env
srv.files = $$SERVICES_CONF_FILES
srv.path  = $$(BUILD_DIR)/etc/meteo/services.conf.d
!win32 { srv.extra = chmod a+r $$SERVICES_CONF_FILES }
INSTALLS += srv


rte.token = env
rte.files = $$ROUTE_FILES
rte.path  = $$(BUILD_DIR)/etc/meteo/route.conf.d
!win32 { app.extra = chmod a+r $$ROUTE_FILES }
INSTALLS += rte

include( $$(SRC)/include.pro )
