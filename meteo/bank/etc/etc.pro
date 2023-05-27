TEMPLATE = lib
TARGET = bank.etc

DB_CONF_FILES       = db.conf.d/*.conf
SERVICES_CONF_FILES = services.conf.d/*.conf

CLEANER_FILES = cleaner.conf.d/*.conf
APP_FILES     = app.conf.d/*.conf
OTHER_FILES   = \
  $$APP_FILES \
  $$DB_CONF_FILES \
  $$SERVICES_CONF_FILES \
  $$CLEANER_FILES

app.token = env
app.files = $$APP_FILES
app.path  = $$(BUILD_DIR)/etc/meteo/app.conf.d
!win32 { app.extra = chmod a+r $$APP_FILES }
INSTALLS += app

db.token = env
db.files = $$DB_CONF_FILES
db.path  = $$(BUILD_DIR)/etc/meteo/db.conf.d
!win32 { app.extra = chmod a+r $$DB_CONF_FILES }
INSTALLS += db

srv.token = env
srv.files = $$SERVICES_CONF_FILES
srv.path  = $$(BUILD_DIR)/etc/meteo/services.conf.d
!win32 { app.extra = chmod a+r $$SERVICES_CONF_FILES }
INSTALLS += srv

clean.token = env
clean.files = $$CLEANER_FILES
clean.path  = $$(BUILD_DIR)/etc/meteo/cleaner.conf.d
!win32 { clean.extra = chmod a+r $$CLEANER_FILES }
INSTALLS += clean

include( $$(SRC)/include.pro )
