TEMPLATE = subdirs
SUBDIRS = \
  services \
  ../commons/ui/isoline/settings.pro\ #TODO: move to commons project
  ui/plugins/settingsplugin\
  ../commons/ui/wrb/map\ #TODO: move to commons project
  ../commons/ui/city/cityplugin.pro\ #TODO: move to commons project
  ui/plugins/cityplugin\
#  ui/plugins/station\
#  ui/plugins/puansoneditor\
  ../commons/ui/plottingwidget\ #TODO: move to commons project
  ui/plugins/plottingplugin\
  etc