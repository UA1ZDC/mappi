QT_VERS=$$[QT_VERSION]
ENV_QTV = $$(QT_VERSION)
PROTOC_VER = $$(PROTOC_VER)

CONFIG += debug
CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT
QT     += xml network concurrent

PROTOPATH += $(BUILD_DIR)/include
LIBS += -L$(BUILD_DIR)/lib

INCLUDEPATH += $$(SRC)
INCLUDEPATH += $$(SRC)/sql/nosql/libmongoc
INCLUDEPATH += $$(SRC)/sql/nosql/libmongoc/mongoc
INCLUDEPATH += $$(SRC)/sql/nosql/libmongoc/libbson
INCLUDEPATH += $$(SRC)/sql/nosql/libmongoc/libbson/bson

QMAKE_CXXFLAGS += -Werror
QMAKE_CXXFLAGS += -Wno-error=pragmas
QMAKE_CXXFLAGS += -Wno-error=deprecated-copy
QMAKE_CXXFLAGS += -Wno-error=deprecated-declarations
QMAKE_CXXFLAGS += -Wno-error=parentheses
QMAKE_CXXFLAGS += -Wno-class-memaccess
#QMAKE_CXXFLAGS += -std=c++17
QMAKE_CXXFLAGS += -std=c++14
QMAKE_CXXFLAGS += -D_REENTRANT

QMAKE_CFLAGS += -Werror
QMAKE_CFLAGS += -Wno-error=pragmas

#contains( ENV_QTV, QT4 ) {
#  QMAKE_CXXFLAGS += -Wno-error=strict-overflow
#}

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

contains( TEMPLATE, sys ){
  TEMPLATE = app
  isEmpty( DESTDIR ):DESTDIR = $(BUILD_DIR)/sbin
}
contains( TEMPLATE, test ){
  TEMPLATE = app
  isEmpty( DESTDIR ):DESTDIR = ./
}
contains( TEMPLATE, autotest ){
  TEMPLATE = app
  isEmpty( DESTDIR ):DESTDIR = ./
}
isEmpty( DESTDIR ) {
  contains( TEMPLATE, app ):DESTDIR = $(BUILD_DIR)/bin
  contains( TEMPLATE, lib ):DESTDIR = $(BUILD_DIR)/lib
}
contains( TEMPLATE, subdirs ):DESTDIR = ./

isEmpty(QT_VERS) {
  QMAKE_UIC = uic -L $(BUILD_DIR)/lib/
}


!isEmpty( QT_VERS ) {
QMAKE_CXXFLAGS += -Wextra -Wformat=2 -Winit-self        \
  -Wundef \
  -Wcast-qual
#-Wunreachable-code
#-Warray-bounds -Wdiv-by-zero
#-Wempty-body
#-Wshadow
#-Wold-style-cast
#-Wconversion -Wfloat-equal  -Waggregate-return
}

win32 {
  !contains( TEMPLATE, app ):CONFIG += dll

  CONFIG -= warn_on

  CONFIG -= debug_and_release
  CONFIG -= debug_and_release_target

  CONFIG += console
  CONFIG -= app_bundle

  QMAKE_CXXFLAGS -= -pthread
  QMAKE_CXXFLAGS -= -Werror
  QMAKE_CXXFLAGS += -Wno-undef

  QMAKE_CFLAGS -= -Werror

  QMAKE_LFLAGS += -Wl,--enable-auto-import

  DEFINES += WIN32
  DEFINES += PROTOBUF_USE_DLLS
}

unix {
  CONFIG += warn_on
  QMAKE_CXXFLAGS += -pthread
  QMAKE_LFLAGS += -Wl,--no-as-needed  

  exists( /usr/bin/pg_config   ):INCLUDEPATH += $$system( pg_config --includedir )
  exists( /usr/include/geotiff ):INCLUDEPATH += /usr/include/geotiff
  exists( /usr/bin/python3.5-config ):QMAKE_CXXFLAGS += $$system( python3.5-config --includes )
  exists( /usr/bin/python3.7-config ):QMAKE_CXXFLAGS += $$system( python3.7-config --includes )
  exists( /usr/bin/python3.8-config ):QMAKE_CXXFLAGS += $$system( python3.8-config --includes )
  exists( /usr/bin/python3.9-config ):QMAKE_CXXFLAGS += $$system( python3.9-config --includes )

  DEFINES += PYTHON3_PATH=\\\"$$system( which python3)\\\"
  DEFINES += GUISU_PATH=\\\"$$(TBS_GUISU)\\\"

  _UNAME_ = $$(_UNAME_)
  isEmpty(_UNAME_):_UNAME_ = $$lower( $$system( uname -a ) )
  contains( _UNAME_, ".*astra.*" ) {
    T_OS_ASTRA_LINUX = 1
    DEFINES += T_OS_ASTRA_LINUX
    L_BOOST_PYTHON = -lboost_python-py35
  }

  isEmpty(T_OS_ASTRA_LINUX) {
    L_BOOST_PYTHON = -lboost_python

    QMAKE_CXXFLAGS += -Wno-implicit-fallthrough
#   QMAKE_CXXFLAGS += -Wno-error=deprecated-copy
    QMAKE_CXXFLAGS += -Wno-error=sign-compare
    QMAKE_CXXFLAGS  += -Wno-error=cast-function-type
    QMAKE_CXXFLAGS += -Wno-aligned-new
    QMAKE_CXXFLAGS += -Wno-error=register    
  }

  # fix bug for gcc 6
  MY_EXTERNAL_INCLUDES -= $$QMAKE_DEFAULT_INCDIRS
  INCLUDEPATH += $$MY_EXTERNAL_INCLUDES
}

contains( CONFIG, qt ) {
    MOC_DIR = ./.moc
    UI_DIR  = ./.ui
    RCC_DIR = ./.rcc
    INCLUDEPATH += ./.ui
}
OBJECTS_DIR = ./.o

STATIC_BUILD=$$(STATIC_BUILD)
!isEmpty( STATIC_BUILD ){
  !contains( TEMPLATE, app ):CONFIG += static
}


# --- INSTALL ---
contains( ENV_QTV, QT3 ) {
  !isEmpty( PUB_HEADERS ){
    headers.files = $$PUB_HEADERS
    headers.path  = $$(BUILD_DIR)/include
    !win32 {headers.extra = chmod a+r $$PUB_HEADERS}
    INSTALLS += headers
  }
}
contains( ENV_QTV, QT4 ):!isEmpty( PUB_HEADERS ){
  #message($$TARGET)
  #message($$_PRO_FILE_PWD_)
  #message($$replace(_PRO_FILE_PWD_, $$PWD, ''))

  INCLUDEPATH += $(BUILD_DIR)/include

  IPATH = $$replace(_PRO_FILE_PWD_, $$PWD, '')

  headers.path  = $$(BUILD_DIR)/include/$$IPATH
  headers.files = $$PUB_HEADERS
  !win32 {headers.extra = chmod a+r $$PUB_HEADERS}
  INSTALLS += headers

  T_LEGACY_MODE = $$(T_LEGACY)
  !isEmpty( T_LEGACY_MODE ){
    legacyHeaders.path  = $$(BUILD_DIR)/include/
    legacyHeaders.files = $$PUB_HEADERS
    !win32 {legacyHeaders.extra = chmod a+r $$PUB_HEADERS}
    INSTALLS += legacyHeaders
  }
}
contains( ENV_QTV, QT5 ):!isEmpty( PUB_HEADERS ){
  #message($$TARGET)
  #message($$_PRO_FILE_PWD_)
  #message($$replace(_PRO_FILE_PWD_, $$PWD, ''))

  INCLUDEPATH += $(BUILD_DIR)/include

  IPATH = $$replace(_PRO_FILE_PWD_, $$PWD, '')

  headers.path  = $$(BUILD_DIR)/include/$$IPATH
  headers.files = $$PUB_HEADERS
  !win32 {headers.extra = chmod a+r $$PUB_HEADERS}
  INSTALLS += headers

  T_LEGACY_MODE = $$(T_LEGACY)
  !isEmpty( T_LEGACY_MODE ){
    legacyHeaders.path  = $$(BUILD_DIR)/include/
    legacyHeaders.files = $$PUB_HEADERS
    !win32 {legacyHeaders.extra = chmod a+r $$PUB_HEADERS}
    INSTALLS += legacyHeaders
  }
}

!isEmpty(PYMODULES){
  pymodules.files = $$PYMODULES
  pymodules.path  = $$(BUILD_DIR)/include.py
  !win32 {pymodules.extra = chmod a+r $$PYMODULES}
  INSTALLS += pymodules
}

# --- SYSLOG ---
win32 {
  INCLUDEPATH  += $$(SYSLOG_INCLUDE)
  QMAKE_LIBDIR += $$(SYSLOG_LIB)
  LIBS += -lsyslog-0
}

# --- PGSQL ---
win32{
  INCLUDEPATH  += $$(PGSQL_INCLUDE)
  QMAKE_LIBDIR += $$(PGSQL_LIB)
  DEFINES+=HAVE_STRUCT_TIMESPEC
}

# --- PROTOBUF ---
win32 {
  INCLUDEPATH  += $$(PROTOBUF_INCLUDE)
  QMAKE_LIBDIR += $$(PROTOBUF_LIB)
  PROTOPATH    += $$(PROTOBUF_INCLUDE)
}

# --- PNG ---
win32:contains( LIBS, "-lpng12" ){
  INCLUDEPATH  += $$(PNG_INCLUDE)
  QMAKE_LIBDIR += $$(PNG_LIB)
}

# --- JASPER ---
win32:contains( LIBS, "-ljasper" ){
  INCLUDEPATH  += $$(JASPER_INCLUDE)
  QMAKE_LIBDIR += $$(JASPER_LIB)
}

# --- FFTW ---
win32:contains( LIBS, "-lfftw" ){
  INCLUDEPATH  += $$(FFTW_INCLUDE)
  QMAKE_LIBDIR += $$(FFTW_LIB)
}
win32:contains( LIBS, "-lrfftw" ){
  INCLUDEPATH  += $$(RFFTW_INCLUDE)
  QMAKE_LIBDIR += $$(RFFTW_LIB)
}

# --- tcustomplot
contains( LIBS, "-ltcustomplot" ){
  DEFINES += QCUSTOMPLOT_USE_LIBRARY
}

# use TARGET value as default facility for log
isEmpty( LOG_MODULE_NAME ){
  LOG_MODULE_NAME = $$TARGET
}
# module name - facility for log
contains( ENV_QTV, QT5 ):DEFINES += LOG_MODULE_NAME=\\\"$$LOG_MODULE_NAME\\\"
contains( ENV_QTV, QT4 ):DEFINES += LOG_MODULE_NAME=\\\"$$LOG_MODULE_NAME\\\"
contains( ENV_QTV, QT3 ):DEFINES += LOG_MODULE_NAME=\"$$LOG_MODULE_NAME\"
contains( ENV_QTV, QT3 ):DEFINES += QT_NO_COMPAT
contains( ENV_QTV, QT3 ):contains( TEMPLATE, lib ):TARGET = $${TARGET}-qt3

DEFINES += PROTOBUF_INLINE_NOT_IN_HEADERS=0


# ----------------------------------------------------------------------------------------------------------------------
# Поддержка токенезированных файлов конфигурации.
#
# Чтобы включить обработку необходимо указать способ обработки для атрибута token.
#   env      - включает использование значений из переменных окружения, начинающихся на TBS_ (а также PRJ_DIR)
#   propfile - включает использование значений из prop-файла
#
# Название файла свойств определяется переменной окружения PROP_NAME
# По умолчанию для linux будет использовать файл linux.prop, для windows win.prop
#
# Пример:
#   etc.token = env propfile
#   etc.files = $$ETC_FILES
#   etc.path  = $$(BUILD_DIR)/etc/ukaz
#   INSTALLS += etc
#
# Запуск обработки:
#   1) qmake - копирует файлы подлежащие обработке в .tbs_props
#   2) make install
#
PROP_NAME = $$(PROP_NAME)
isEmpty( PROP_NAME ):win32:PROP_NAME=win
isEmpty( PROP_NAME ):unix:PROP_NAME=linux

# очищаем временную директорию
#system( rm -r $$_PRO_FILE_PWD_/.tbs_props )

for( i, INSTALLS ){
  _TOK_OPT = $$eval($${i}.token)

  !isEmpty( _TOK_OPT ){
#    message( "Обработка $${i}" )

    _TOK_FILES = $$eval($${i}.files)
    _TOK_PATH  = $$eval($${i}.path)
    _TOK_TMP   = $$_PRO_FILE_PWD_/.tbs_props
    system( test -d $$_TOK_TMP || mkdir $$_TOK_TMP )

    #message( "Файлы $${_TOK_FILES}" )

    # очищаем список файлов
    eval($${i}.files = "")

    # копируем файлы во временную директорию
    for( f, _TOK_FILES ){
      _f_name = $$basename(f) # имя файла
      _f_dir  = $$dirname(f)  # путь к файлу

      _tmp_dir = $${_TOK_TMP}/$${_f_dir} # путь к файлу во временной директории

      # создаём временную директорию
      system( test -d $$_tmp_dir || mkdir -p $$_tmp_dir )
      # копируем файлы
      system( cp -fpr $$_PRO_FILE_PWD_/$${_f_dir}/$${_f_name} $$_TOK_TMP/$${_f_dir}/ )

      contains( _TOK_OPT, "env" ){
        _setprop_opt = "--env "
      }
      contains( _TOK_OPT, "propfile" ){
        isEmpty( _prop_path ):_prop_file = $${_PRO_FILE_PWD_}/props/$${PROP_NAME}.prop

        !exists( $${_prop_file} ){ error("Отсутствует файл параметров $${_prop_file}") }
        _setprop_opt += "--prop=$${_prop_file} "
      }

      # обработка файла
      !isEmpty( _setprop_opt ):system( $$(SRC)/build_tools/tool_setprop.sh $${_setprop_opt} --inst=$${_tmp_dir}/ --orig=$${_PRO_FILE_PWD_} -- $${f} )

      # подменяем директорию, из которой будут браться файлы для установки
      eval( $${i}.files += $${_TOK_TMP}/$${f} )
    }
  }
}
# ----------------------------------------------------------------------------------------------------------------------

HEADERS += $$PUB_HEADERS
HEADERS += $$PRIVATE_HEADERS

# Дополнительные опции для очистки - tbs(токенизированные конфиги), .moc, .obj, .ui файлы
cleantbs.target = cleantbs
cleantbs.commands = if [ -d '.tbs_props' ] ; then rm -r '.tbs_props'; fi

cleanmoc.target = cleanmoc
cleanmoc.commands = if [ -d '.moc' ] ; then rm -r '.moc'; fi

cleanobj.target = cleanobj
cleanobj.commands = if [ -d '.o' ] ; then rm -r '.o'; fi

cleanui.target = cleanui
cleanui.commands = if [ -d '.ui' ] ; then rm -r '.ui'; fi

distclean.depends = cleantbs cleanmoc cleanobj cleanui
QMAKE_EXTRA_TARGETS += distclean cleantbs cleanmoc cleanobj cleanui
