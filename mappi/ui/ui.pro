TEMPLATE = subdirs

UI_DIRS += $$system("ls $$PWD")

for(d, UI_DIRS):exists($$PWD/$$d/$${d}.pro):SUBDIRS += $$PWD/$$d/$${d}.pro
for(d, UI_DIRS):exists($$PWD/$$d/test/$${d}.test.pro):SUBDIRS += $$PWD/$$d/test/$${d}.test.pro

SUBDIRS += pos/satdocument.pro
