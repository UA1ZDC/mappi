TEMPLATE = subdirs
SUBDIRS = \
  cross-commons \ #dependencies
  commons       \ #dependencies
  satellite/spsatellite.pro # TODO: Rename or make subproject

cross-commons.subdir = ../cross-commons
commons.subdir  = ../commons
sat-commons.depends = \
  cross-commons \
  commons