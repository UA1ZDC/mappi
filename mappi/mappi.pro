TEMPLATE = subdirs
SUBDIRS = \
  cross-commons \ # dependencies
  sql           \ # dependencies
  commons       \ # dependencies
  meteo         \ # dependencies
  sat-commons   \ # dependencies
  etc\
  proto\
  settings\
  global\
  appmanager\
  appclient\
  cron\
  cleaner\
  landmask\
  schedule\
  fsm/receiver.pro\ # TODO: Rename or make subproject
  schedule/appmain/app.pro\ # TODO: Rename or make subproject
  pretreatment/savenotify\
  pretreatment/images\
  pretreatment/formats\
  pretreatment/handler\
  pretreatment/fileapp\
  pretreatment/app\
  pretreatment/service\
  device/receiver\
  device/antenna\
  services/schedule\
  services/playback\
  services/receiver\
  services/antenna\
  services/sessiondataservice\
  services/fileservice\
  services/fileservice/fileserviceclient\
  thematic/asmjit\
  thematic/thematiccalc\
  projection/satprojection.pro\ # TODO: Rename or make subproject
  ui/pos/satdocument.pro\ # TODO: Rename or make subproject
  ui/scheduletable\
  ui/antenna\
  ui/exprguid\
  ui/satlayer\
  ui/satelliteimage\
# thematic/algs\
  thematic/algs_calc/algs.pro\ # TODO: Rename or make subproject
#  thematic/app\
  thematic/app_calc/app.pro\ # TODO: Rename or make subproject
  thematic/service\
  ui/plugins/settings/base\
  ui/plugins/settings/receiver\
  ui/plugins/settings/schedule\
  ui/plugins/settings/thematic\
  ui/plugins/settings\
  ui/plugins/antenna/antennaplugin.pro\ # TODO: Rename or make subproject
  ui/plugins/sessions\
  ui/plugins/sessionviewerplugin\
# ui/plugins/uhdcontrol/coordaction\
# ui/plugins/uhdcontrol/valueaction\
# ui/plugins/uhdcontrol\
  ui/plugins/pointvalue\
  ui/plugins/maskslider\
  ui/plugins/montageaction\
  ui/plugins/histogrameditor\
  ui/plugins/brightnessaction\
  ui/plugins/transparencyaction\
# ftploader\
# mmirz\
  meteo-app \ # subprojects
  ui/mainwindow

meteo-app.file = ../meteo/commons/ui/mainwindow/appmain/mainwindow.pro
cross-commons.subdir = ../cross-commons
sql.subdir = ../sql
commons.subdir  = ../commons
meteo.subdir  = ../meteo
sat-commons.subdir = ../sat-commons
mappi.depends = \
  cross-commons \
  sql           \
  commons       \
  meteo         \
  sat-commons

CONFIG += ordered # TODO: One more kitten dies