TEMPLATE = subdirs

DIRS = $$system(ls)

for(d, DIRS):exists($$PWD/$${d}/$${d}.pro):SUBDIRS += $$PWD/$$d/$${d}.pro
for(d, DIRS):exists($$PWD/$${d}/test/$${d}.test.pro):SUBDIRS += $$PWD/$$d/test/$${d}.test.pro
