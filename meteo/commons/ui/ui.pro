TEMPLATE = subdirs

UI_DIRS += $$system("ls $$PWD")

for(d, UI_DIRS):exists($$PWD/$$d/$${d}.pro):SUBDIRS += $$PWD/$$d/$${d}.pro
for(d, UI_DIRS):exists($$PWD/$$d/test/$${d}.test.pro):SUBDIRS += $$PWD/$$d/test/$${d}.test.pro

SUBDIRS +=                                        \
$$PWD/map/view                                    \
$$PWD/map/view/drawtools/map.view.drawtools.pro   \
$$PWD/custom/designerplugin                       \


