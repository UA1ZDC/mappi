TEMPLATE = lib
TARGET = meteo.db

FILES +=        dump

FILES +=        dump.py         \
                cluster.sh      \
                clusterncuo.sh  \
                restore.py      \
                scheme_list.json

inst.files = $$FILES
inst.path = $$(BUILD_DIR)/share/meteo/db

INSTALLS += inst
OTHER_FILES += $$FILES

include( $$(SRC)/include.pro )
