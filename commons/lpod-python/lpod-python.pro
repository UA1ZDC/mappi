######################################################################
# Automatically generated by qmake (2.01a) ?? ??? 5 14:41:09 2017
######################################################################

TEMPLATE = lib

LPOD = lpod/*.py                \

TEMPLATES = lpod/templates/*    \

lpod.files = $$LPOD
lpod.path  = $$(BUILD_DIR)/include.py/lpod
!win32 {lpod.extra += chmod a+r $$LPOD; }
INSTALLS += lpod

templates.files = $$TEMPLATES
templates.path = $$(BUILD_DIR)/include.py/lpod/templates
!win32 {templates.extra += chmod a+r $$TEMPLATES; }
INSTALLS += templates

include( $$(SRC)/include.pro )