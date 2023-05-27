TEMPLATE = lib
TARGET   = commons.installpack

INST_SCRIPTS = preinst postinst
script.token = env
script.files = $$INST_SCRIPTS
script.path = $$(PACKAGE_DIR)/DEBIAN

OTHER_FILES         = $$INST_SCRIPTS 

INSTALLS += script

include( $$(SRC)/include.pro )
