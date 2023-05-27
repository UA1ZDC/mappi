TEMPLATE = lib
TARGET   = prognoz.installpack

INST_SCRIPTS = postinst postrm

OTHER_FILES         = $$INST_SCRIPTS

inst.token = env
inst.files = $$INST_SCRIPTS
inst.path = $$(PACKAGE_DIR)/DEBIAN
INSTALLS += inst


include( $$(SRC)/include.pro )

