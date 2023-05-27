TEMPLATE = subdirs
SUBDIRS = \
  cross-commons \ #subproject
  proto \
  bson \ #subproject
  dbi \
  nosql \
  psql

cross-commons.subdir = ../cross-commons
bson.subdir = nosql/libmongoc/libbson

sql.depends = cross-commons
dbi.depends = cross-commons proto bson
nosql.depends = bson dbi
psql.depends = cross-commons dbi nosql