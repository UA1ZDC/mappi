TEMPLATE = lib
TARGET   = meteo.oceandiag.core
QT += widgets

SOURCES =       oceandiag_doc.cpp           \
                oceandiagdata.cpp           \
                createoceandiagwidget.cpp


HEADERS =       oceandiag_doc.h             \
                oceandiagdata.h             \
                createoceandiagwidget.h

FORMS +=        createoceandiagwidget.ui

LIBS +=                 -lprotobuf              \
                        -ltdebug                \
                        -lmnmathtools           \
                        -ltrpc                  \
                        -ltapp                  \
                        -lobanal                \
                        -lmeteodata             \
                        -locean                 \
                        -ltcustomui             \
                        -lmeteo.sql              \
                        -lmeteo.mainwindow      \
                        -lmeteo.geobasis        \
                        -lmeteo.proto           \
                        -lmeteo.global          \
                        -lmeteo.settings        \
                        -lmeteo.etc             \
                        -lmeteo.map             \
                        -lmeteo.map.view        \
                        -lmeteo.graph           \
                        -lmeteo.oceantable.core


include( $$(SRC)/include.pro )
