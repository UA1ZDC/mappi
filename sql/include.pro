message("Using sql standalone include.pro")
QT_VERS=$$[QT_VERSION]
ENV_QTV = $$(QT_VERSION)
PROTOC_VER = $$(PROTOC_VER)

CONFIG += debug
CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT

LIBS += -L$(BUILD_DIR)/lib
INCLUDEPATH += $(SRC)
INCLUDEPATH += $(SRC)/sql/nosql/libmongoc
INCLUDEPATH += $(SRC)/sql/nosql/libmongoc/mongoc
INCLUDEPATH += $(SRC)/sql/nosql/libmongoc/libbson
INCLUDEPATH += $(SRC)/sql/nosql/libmongoc/libbson/bson
exists( /usr/bin/pg_config ):INCLUDEPATH += $$system( pg_config --includedir )

QMAKE_CXXFLAGS += -Werror
QMAKE_CXXFLAGS += -std=c++14
QMAKE_CXXFLAGS += -D_REENTRANT
QMAKE_CXXFLAGS += -Wno-error=deprecated-declarations
QMAKE_CXXFLAGS += -Wno-error=deprecated-copy
QMAKE_CXXFLAGS += -Wno-aligned-new

QMAKE_CFLAGS += -Werror
QMAKE_CFLAGS += -Wno-error=pragmas

T_PROJECT_DIR=$$(PRJ_DIR)
contains( TARGET, tapp ){
  !isEmpty( T_PROJECT_DIR ) {
    contains( ENV_QTV, QT3 ):DEFINES += T_PROJECT_DIR=\"$$(PRJ_DIR)/\"
    contains( ENV_QTV, QT4 ):DEFINES += T_PROJECT_DIR=\\\"$$(PRJ_DIR)/\\\"
    contains( ENV_QTV, QT5 ):DEFINES += T_PROJECT_DIR=\\\"$$(PRJ_DIR)/\\\"
  }
  else {
    contains( ENV_QTV, QT3 ):DEFINES += T_PROJECT_DIR=\"build_dir/\"
    contains( ENV_QTV, QT4 ):DEFINES += T_PROJECT_DIR=\\\"build_dir/\\\"
    contains( ENV_QTV, QT5 ):DEFINES += T_PROJECT_DIR=\\\"build_dir/\\\"
  }
}

isEmpty( DESTDIR ) {
  contains( TEMPLATE, app ):DESTDIR = $(BUILD_DIR)/bin
  contains( TEMPLATE, lib ):DESTDIR = $(BUILD_DIR)/lib
}
contains( TEMPLATE, subdirs ):DESTDIR = ./

isEmpty( LOG_MODULE_NAME ){
  LOG_MODULE_NAME = $$TARGET
}
# module name - facility for log
contains( ENV_QTV, QT5 ):DEFINES += LOG_MODULE_NAME=\\\"$$LOG_MODULE_NAME\\\"
contains( ENV_QTV, QT4 ):DEFINES += LOG_MODULE_NAME=\\\"$$LOG_MODULE_NAME\\\"
contains( ENV_QTV, QT3 ):DEFINES += LOG_MODULE_NAME=\"$$LOG_MODULE_NAME\"
contains( ENV_QTV, QT3 ):DEFINES += QT_NO_COMPAT
contains( ENV_QTV, QT3 ):contains( TEMPLATE, lib ):TARGET = $${TARGET}-qt3