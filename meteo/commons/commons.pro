TEMPLATE = subdirs
SUBDIRS = \
  cross-commons \
  sql           \
  top-commons \
  proto  \
  auth\
  rpc\
  rpcpywrap \
  settings\
  punchrules\
  websockets\
  global\
  globalwrap\
  formsh\
  fonts\
  xmlproto\
  radar\
  zond\
  zond/zondwrap\
  ocean\
  appmanager  \
  services/sprinf\
  grib\
  grib/parser\
  grib/iface\
  primarydb\
  services/obanal\
  services/obanal/appmain/obanal.appmain.pro\ # TODO: Rename or make subproject
  ui/map/tileimage\
  ui/map\
  rc\
  ui/custom\
  ui/mainwindow/embedwindow\
  ui/mainwindow\
  ui/viewheader/maslo.viewheader.pro\ # TODO: Rename or make subproject
  ui/appclient\
  ui/map/view/custom\
  ui/map/view\
  ui/map/loader/old.geo/oldgeo.pro\ # TODO: Rename or make subproject
  ui/map/loader/ptkpp.geo/ptkppgeo.pro\ # TODO: Rename or make subproject
  ui/conditionmaker/condition.pro\ # TODO: Rename or make subproject
  ui/map/dataexchange\
  ui/map/puansonwrap\
  ui/map/weatherwrap\
  tlgpool\
  services/decoders\
  services/state    \
  control/meteocontrol.pro\ # TODO: Rename or make subproject
  alphanum\
  bufr\
  grib/decoder\
  msgparser\
  msgdata\
  services/msgcenter\
  services/fieldata\
  services/srcdata\
  services/map/service.pro\ # TODO: Rename or make subproject
  services/formalpy\
# msgcenter/proto\
  services/indexes\
  services/obanal_indexes/obanalindexes.pro\ # TODO: Rename or make subproject
  planner  \
  services/map/tool\
  services/map/runjob\
  astrodata\
  astrodata/window/astrodatawindow.pro\ # TODO: Rename or make subproject
  astrodata/astrodatawrap\
  services/forecast\
  services/forecast/service/forecast.pro\ # TODO: Rename or make subproject
  services/forecast/process/forecast.process.pro\ # TODO: Rename or make subproject
  services/forecast/opravd\
  services/climatsaver\
  services/climatdata\
  services/vko\
  services/esimo_nc\
  faxes  \
  msgstream\
  msgstream/plugins/filestream\
  msgstream/plugins/ftpstream\
  msgstream/plugins/sriv512  \
  msgstream/plugins/ptkpp\
  msgstream/plugins/socketspecial\
  msgstream/plugins/socketspec2g\
  msgstream/plugins/udpstream\
  msgstream/plugins/httpstream\
  msgstream/plugins/cliwarestream\
  msgstream/plugins/esimostream\
  msgstream/appmain/msgstream.main.pro\ # TODO: Rename or make subproject
  cleaner\
  cron    \
  appclient  \
  createanimation/createanime.pro\ # TODO: Rename or make subproject
  services/chainloader\
  ui/customviewer/decode/decodeviewer.pro\ # TODO: Rename or make subproject
  services/customviewer/customviewer.service.pro\ # TODO: Rename or make subproject
  ui/obanalsettings/timesheeteditor\
  ui/obanalsettings\
  ui/graph\
  ui/graphitems\
  ui/plugins/axisaction\
  ui/graph/coordaction/graph.coordaction.pro \ # TODO: Rename or make subproject
  ui/graph/valueaction/graph.valueaction.pro \ # TODO: Rename or make subproject
  ui/map/view/drawtools/map.view.drawtools.pro\ # TODO: Rename or make subproject
  ui/plugins/drawtools\
  ui/plottingwidget\
  ui/plugins/navigator\
  ui/plugins/fields/fieldsaction.pro\ # TODO: Rename or make subproject
  ui/plugins/ruler/ruleraction.pro\ # TODO: Rename or make subproject
  ui/plugins/nabludenia  \
#  ui/plugins/puanson\
#  ui/plugins/opensavedoc\
  ui/plugins/renamedoc/plugin.pro  \ # TODO: Rename or make subproject
  ui/stationlist\
  ui/aero/table/aerotable.pro\ # TODO: Rename or make subproject
  ui/aero/verticalcut\
  ui/aero\
  ui/plugins/aeroplugin\
  ui/plugins/verticalplugin/verticalcutplugin.pro \ # TODO: Rename or make subproject
  etc

cross-commons.subdir = ../../cross-commons
sql.subdir = ../../sql
top-commons.subdir  = ../../commons
commons.depends = cross-commons sql top-commons
rpc.subdir = rpc
rpc.depends = top-commons cross-commons
rpcpywrap.file = rpc/pywrap/rpcpywrap.pro
rpcpywrap.depends = rpc proto
settings.depends = top-commons cross-commons sql
punchrules.depends = top-commons cross-commons
global.depends = top-commons cross-commons sql rpc websockets
globalwrap.subdir = global/globalwrap
globalwrap.depends = global settings sql
formsh.depends = top-commons cross-commons

CONFIG += ordered # TODO: One more kitten dies