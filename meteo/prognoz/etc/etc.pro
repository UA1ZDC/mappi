TEMPLATE = lib
TARGET = prognoz.etc

DB_CONF_FILES         = db.conf.d/*.conf
SERVICES_CONF_FILES   = services.conf.d/*.conf
MAINWINDOW_CONF_FILES = mainwindow.menu/*.conf

APP_FILES   = app.conf.d/*.conf
CRON_FILES  = cron.d/*.conf
OTHER_FILES = \
  $$APP_FILES \
  $$DB_CONF_FILES \
  $$SERVICES_CONF_FILES \
  $$MAINWINDOW_CONF_FILES \
  $$CRON_FILES

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

crn.token = env
crn.files = $$CRON_FILES
crn.path  = $$(BUILD_DIR)/etc/meteo/cron.d
!win32 { app.extra = chmod a+r $$CRON_FILES }
INSTALLS += crn

mainwindow.token = env
mainwindow.files = $$MAINWINDOW_CONF_FILES
mainwindow.path  = $$(BUILD_DIR)/etc/meteo/mainwindow.menu
!win32 { app.extra = chmod a+r $$MAINWINDOW_CONF_FILES  }
INSTALLS += mainwindow

include( $$(SRC)/include.pro )
