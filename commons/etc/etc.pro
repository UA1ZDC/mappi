TEMPLATE = lib
TARGET   = commons.etc

FIELD_FILES  = field.conf.d/*.conf

include( $$(SRC)/include.pro )


field.files = $$FIELD_FILES
field.path  = $$(BUILD_DIR)/etc/meteo/field.conf.d
!win32 {field.extra = chmod a+r $$FIELD_FILES}

INSTALLS += field

