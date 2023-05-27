TEMPLATE = subdirs
SUBDIRS = \
  cross-commons \ #subproject
  sql-proto \ #subproject
  etc \
  textproto \
  proto \
#  testhelper \
#  defines \
  container \
  mathtools \
  funcs \
  coord_systems \
  geobasis   \
  geom \
  meteo_data \
  meteodatawrap \
  landmask \
  obanal \
#  factory \
#  xmlparser \
#  settings \
  sun_moon \
#  compress \
  compresslib \ # TODO: Rename or make subproject
  ui/qcustomplot \
  proc_read \
  lpod-python \
  lpod-python3/lpod-python.pro \ # TODO: Rename or make subproject
#  cgal \
  qftp

cross-commons.subdir = ../cross-commons
sql-proto.subdir = ../sql/proto
commons.depends = cross-commons sql-proto
textproto.depends = cross-commons
mathtools.file = mathtools/spmathtools.pro
funcs.file = funcs/mnfuncs.pro
funcs.depends = mathtools cross-commons
coord_systems.file = coord_systems/spcoordsys.pro
coord_systems.depends = mathtools funcs
geobasis.file = geobasis/basis.pro
geobasis.depends = cross-commons mathtools coord_systems
geom.depends = cross-commons
meteo_data.depends = cross-commons geobasis mathtools
meteodatawrap.subdir = meteo_data/meteodatawrap
meteodatawrap.depends = meteo_data
landmask.depends = cross-commons geobasis mathtools
obanal.depends = cross-commons mathtools geobasis proto textproto
sun_moon.depends = mathtools
compresslib.file = compresslib/compress.pro
compresslib.depends = cross-commons
proc_read.depends = cross-commons