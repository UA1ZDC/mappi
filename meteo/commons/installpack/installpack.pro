TEMPLATE = lib
TARGET   = meteo.commons.installpack

LD_PATH_FILE=meteo.ld.conf
ldconf.token = env
ldconf.files = $$LD_PATH_FILE
ldconf.path  = $$(PACKAGE_DIR)/etc/ld.so.conf.d/
INSTALLS += ldconf
OTHER_FILES += $$LD_PATH_FILE

INST_SCRIPTS = postinst postrm prerm
inst.token = env
inst.files = $$INST_SCRIPTS
inst.path = $$(PACKAGE_DIR)/DEBIAN
INSTALLS += inst
OTHER_FILES += $$$$INST_SCRIPTS 

LIMITS_FILES = meteo.limits.conf
limits.files = $$LIMITS_FILES
limits.path = $$(PACKAGE_DIR)/etc/security/limits.d
INSTALLS += limits
OTHER_FILES += $$LIMITS_FILES

include( $$(SRC)/include.pro )
