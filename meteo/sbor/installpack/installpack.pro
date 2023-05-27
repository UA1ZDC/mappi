TEMPLATE = lib
TARGET   = sbor.installpack

MENU_FILES =  mappi.msgloader.desktop

OTHER_FILES = $$MENU_FILES

menu.token = env
menu.files = $$MENU_FILES
menu.path = $$(PACKAGE_DIR)/usr/share/applications/mappi
INSTALLS += menu

include( $$(SRC)/include.pro )

