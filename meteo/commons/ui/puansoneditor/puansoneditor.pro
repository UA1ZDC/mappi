TEMPLATE = lib
TARGET   = meteo.puansoneditor
QT += widgets

FORMS =         puanson.ui              \
                metaedit.ui             \
                fontdialog.ui           \
                numformat.ui            \
                symbolformat.ui         \
                dlgspinbox.ui           \
                addsymbol.ui            \
                punchselect.ui

PRIVATE_HEADERS =       editor.h                \
                        metaedit.h              \
                        puansonitem.h           \
                        numformat.h             \
                        symbolformat.h          \
                        dlgspinbox.h            \
                        addsymbol.h             \
                        gv.h                    \
                        punchselect.h

SOURCES =               editor.cpp              \
                        metaedit.cpp            \
                        puansonitem.cpp         \
                        numformat.cpp           \
                        symbolformat.cpp        \
                        dlgspinbox.cpp          \
                        addsymbol.cpp           \
                        gv.cpp                  \
                        punchselect.cpp

LIBS +=         -ltsingleton            \
                -lmeteodata             \
                -lprotobuf              \
                -ltcustomui             \
                -lmeteo.textproto       \
                -lmeteo.map             \
                -lmeteo.etc             \
                -lmeteo.font            \
                -lmeteo.proto

LIBS +=                 -ltapp                          \
                        -ltdebug                        \
                        -ltrpc                          \
                        -lmeteo.proto                   \
                        -lmeteo.map                     \
                        -lmeteo.map.view                \
                        -lmeteo.global                  \
                        -lmeteo.geobasis                \
                        -lmeteodata                     \
                        -lmeteo.sql                      \
                        -lmeteo.mainwindow              \
                        -lobanal                        \
                        -ltcustomui                     \
                        -lmnmathtools
                
include( $$(SRC)/include.pro )

RESOURCES += \
    ../../rc/meteo.icons.qrc
