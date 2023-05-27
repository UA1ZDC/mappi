TEMPLATE    = lib
TARGET      = meteo.app.settings

QT += widgets

LOG_MODULE_NAME = settings

PUB_HEADERS = settingsform.h
QT += script

HEADERS     = settingswidget.h        \
              appstartsettings.h      \
              dbsettings.h            \
              locsettings.h           \
              msgsettings.h           \
              dblist.h                \
              stationsearch.h         \
              settingsfuncs.h         \
              datasortitem.h          \
              climatsettings.h        \
              stormsettings.h         \
              pogodainputsettings.h   \
              stationselectform.h     \
              appmain.h               \
              pogodainputsettingsperuser.h

SOURCES     = settingsform.cpp        \
              settingswidget.cpp      \
              appstartsettings.cpp    \
              dbsettings.cpp          \
              locsettings.cpp         \
              msgsettings.cpp         \
              appmain.cpp             \
              dblist.cpp              \
              stationsearch.cpp       \
              settingsfuncs.cpp       \
              datasortitem.cpp        \
              climatsettings.cpp      \
              stormsettings.cpp       \
              pogodainputsettings.cpp \
              stationselectform.cpp   \
    pogodainputsettingsperuser.cpp

FORMS       = settings.ui             \
              appstartsettings.ui     \
              dbsettings.ui           \
              locsettings.ui          \
              msgsettings.ui          \
              stationsearch.ui        \
              dblist.ui               \              
              stationselectform.ui    \
    pogodainputsettingsperuser.ui

LIBS       += -lmeteo.etc             \
              -lmeteo.appsettings     \
              -lmeteo.geobasis        \
              -lmeteo.global          \
              -lmeteo.map             \
              -lmeteo.map.oldgeo      \
              -lmeteo.proto           \
              -lmeteo.settings        \
              -lmeteo.textproto       \
              -lmnmathtools           \
              -lprotobuf              \
              -lmeteo.sql              \
              -lspcoordsys            \
              -ltapp                  \
              -ltcustomui             \
              -ltdebug                \
              -ltrpc                  \
              -ltxmlproto             \
              -lmeteo.map.dataexchange\
              -lmeteo.map.ptkpp       \
              -lmeteo.tileimage       \
              -lmeteo.mainwindow      \
              -lmeteo.map             \
              -lmeteo.map.view

INCLUDEPATH += $$(SRC)/meteo/commons/rpc \
               $$(SRC)/meteo/commons/proto

include( $$(SRC)/include.pro )
