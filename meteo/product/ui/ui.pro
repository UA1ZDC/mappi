TEMPLATE = subdirs

DIR       = $$(SRC)/meteo/product/ui
PRJ_DIRS += $$system("ls $$DIR")

for(d, PRJ_DIRS):exists($$DIR/$${d}/$${d}.pro):SUBDIRS += $$DIR/$$d/$${d}.pro
for(d, PRJ_DIRS):exists($$DIR/$${d}/app/$${d}.app.pro):SUBDIRS += $$DIR/$$d/app/$${d}.app.pro
for(d, PRJ_DIRS):exists($$DIR/$${d}/test/$${d}.test.pro):SUBDIRS += $$DIR/$$d/test/$${d}.test.pro
for(d, PRJ_DIRS):exists($$DIR/$${d}/autotest/$${d}.autotest.pro):SUBDIRS += $$DIR/$$d/autotest/$${d}.autotest.pro
