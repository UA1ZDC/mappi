TEMPLATE = lib
TARGET   = meteo.etc

FONT_FILES = fonts/*.ttf

CITY_FILES = geo/*.pbf

PTKPP_GEO  = geo/sxf.map_ptkpp \
             geo/vo.map_ptkpp  \
             geo/rf.map_ptkpp

RESOURCES =  meteo.icons.qrc   \
             meteo.css.qrc     \
             meteo.geoicons.qrc\
             html.qrc
             
ICON_FILE    =  icons/icon-64.png \
	        icons/backup.png \
		icons/msgloader.png \
		icons/settings-2.png \
		icons/control.png
                
icon.files  = $$ICON_FILE
icon.path   = $$(BUILD_DIR)/share/meteo
INSTALLS   += icon

city.files = $$CITY_FILES
city.path  = $$(BUILD_DIR)/share/meteo/geo
INSTALLS += city

ptkpp.files = $$PTKPP_GEO
ptkpp.path  = $$(BUILD_DIR)/share/meteo/geo
INSTALLS += ptkpp

font.files = fonts/*.ttf
font.path = $$(BUILD_DIR)/share/meteo/fonts
INSTALLS += font

include( $$(SRC)/include.pro )
