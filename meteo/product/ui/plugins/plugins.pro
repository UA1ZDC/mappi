TEMPLATE = subdirs

PLG_DIRS += $$system("ls $$PWD")

for(d, PLG_DIRS):exists($$PWD/$$d/$${d}.pro):SUBDIRS += $$PWD/$$d/$${d}.pro
for(d, PLG_DIRS):exists($$PWD/$$d/test/$${d}.test.pro):SUBDIRS += $$PWD/$$d/test/$${d}.test.pro

