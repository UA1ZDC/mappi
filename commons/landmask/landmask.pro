TEMPLATE = lib
TARGET   = meteo.landmask

SOURCES =   landmask.cpp          \
            elevation.cpp

HEADERS =   landmask.h            \
            elevation.h

LIBS +=     -ltapp                \
            -lmnmathtools         \
            -lmeteo.geobasis

LANDMASK_FILES = data/*.dat
landmask.files = $$LANDMASK_FILES
landmask.path  = $$(BUILD_DIR)/share/meteo/landmask
INSTALLS += landmask

ATLAS_FILES = $$(SRC)/data/atlas/*.dat
atlas.files = $$ATLAS_FILES
atlas.path  = $$(BUILD_DIR)/share/meteo/landmask/atlas
INSTALLS += atlas

GLOBE_LANDMASK_FILES = $$(SRC)/bindata/globe/a11g \
                       $$(SRC)/bindata/globe/b10g \
                       $$(SRC)/bindata/globe/c10g \
                       $$(SRC)/bindata/globe/d10g \
                       $$(SRC)/bindata/globe/e10g \
                       $$(SRC)/bindata/globe/f10g \
                       $$(SRC)/bindata/globe/g10g \
                       $$(SRC)/bindata/globe/h10g \
                       $$(SRC)/bindata/globe/i10g \
                       $$(SRC)/bindata/globe/j10g \
                       $$(SRC)/bindata/globe/k10g \
                       $$(SRC)/bindata/globe/l10g \
                       $$(SRC)/bindata/globe/m10g \
                       $$(SRC)/bindata/globe/n10g \
                       $$(SRC)/bindata/globe/o10g \
                       $$(SRC)/bindata/globe/p10g

globe_comp.name = globe

for( f, GLOBE_LANDMASK_FILES ){
  !exists( $$f ){
    warning( "Не найден файл $$f" )
  }
}

globe_comp.input = GLOBE_LANDMASK_FILES
globe_comp.output = $$(BUILD_DIR)/share/meteo/landmask/globe/${QMAKE_FILE_IN_BASE}
globe_comp.commands = mkdir -p $$(BUILD_DIR)/share/meteo/landmask/globe/ && test -e "${QMAKE_FILE_OUT}" || cp "${QMAKE_FILE_IN}" "$$(BUILD_DIR)/share/meteo/landmask/globe"
globe_comp.variable_out = GENERATED_FILES
globe_comp.CONFIG += target_predeps
globe_comp.CONFIG += ignore_no_exist
QMAKE_EXTRA_COMPILERS += globe_comp

include( $$(SRC)/include.pro )
