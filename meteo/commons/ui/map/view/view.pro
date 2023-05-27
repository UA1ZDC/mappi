TEMPLATE = lib
TARGET   = meteo.map.view
QT += printsupport

LOG_MODULE_NAME = vizdoc

PUB_HEADERS =           mapscene.h                      \
                        mapview.h                       \
                        mapwindow.h                     \
                        actions/action.h

PRIVATE_HEADERS =       widgetitem.h                    \
                        menu.h                          \
                        actionbutton.h                  \
                        actions/debugaction.h           \
                        actions/hidebuttonsaction.h     \
                        actions/layersaction.h          \
                        actions/scaleaction.h           \
                        actions/translateaction.h       \
                        actions/rotateaction.h          \
                        actions/fanalyseaction.h        \
                        actions/contextmenueventaction.h\
                        actions/focusaction.h           \
                        actions/savesxfaction.h         \
                        actions/savebmpaction.h         \
                        actions/printdocaction.h        \
                        actions/opensxfaction.h         \
                        actions/selectaction.h          \
                        actions/mdiaction.h             \
                        actions/mdioutaction.h          \
                        actions/coordsaction.h          \
                        actions/mousecoordaction.h      \
                        actions/coordproxywidget.h      \
                        actions/incutaction.h           \
                        actions/legendaction.h          \
                        actions/renderitem.h            \
                        actions/informaction.h          \
                        actions/infotextaction.h        \
                        actions/insertaction.h          \
                        actions/traceaction.h           \
                        actions/arrow.h                 \
                        actions/traceitem.h             \
                        actions/labelitem.h             \
                        actions/textitem.h              \
                        widgets/coordwidget.h           \
                        widgets/layerswidget.h          \
                        widgets/mapwidget.h             \
                        widgets/savesxf.h               \
                        widgets/importsxf.h             \
                        widgets/maprubber.h             \
                        widgets/layersselect.h          \
                        widgets/printdoc.h              \
                        widgets/scalewidget.h           \
                        widgets/rotatewidget.h          \
                        widgets/savebitmap.h            \
                        widgets/printpreview.h          \
                        widgets/incutparams.h           \
                        widgets/informwidget.h

SOURCES =               mapscene.cpp                    \
                        mapview.cpp                     \
                        widgetitem.cpp                  \
                        mapwindow.cpp                   \
                        actionbutton.cpp                \
                        menu.cpp                        \
                        actions/action.cpp              \
                        actions/debugaction.cpp         \
                        actions/hidebuttonsaction.cpp   \
                        actions/layersaction.cpp        \
                        actions/scaleaction.cpp         \
                        actions/translateaction.cpp     \
                        actions/rotateaction.cpp        \
                        actions/focusaction.cpp         \
                        actions/fanalyseaction.cpp      \
                        actions/contextmenueventaction.cpp\
                        actions/mdiaction.cpp           \
                        actions/selectaction.cpp        \
                        actions/mdioutaction.cpp        \
                        actions/coordsaction.cpp          \
                        actions/savesxfaction.cpp       \
                        actions/savebmpaction.cpp       \
                        actions/printdocaction.cpp      \
                        actions/opensxfaction.cpp       \
                        actions/mousecoordaction.cpp    \
                        actions/coordproxywidget.cpp    \
                        actions/incutaction.cpp         \
                        actions/legendaction.cpp        \
                        actions/informaction.cpp        \
                        actions/renderitem.cpp          \
                        actions/infotextaction.cpp      \
                        actions/traceaction.cpp   \
                        actions/arrow.cpp         \
                        actions/traceitem.cpp     \
                        actions/labelitem.cpp     \
                        actions/textitem.cpp\
                        widgets/coordwidget.cpp         \
                        widgets/layerswidget.cpp        \
                        widgets/mapwidget.cpp           \
                        widgets/savesxf.cpp             \
                        widgets/importsxf.cpp           \
                        widgets/maprubber.cpp           \
                        widgets/printdoc.cpp            \
                        widgets/savebitmap.cpp          \
                        widgets/printpreview.cpp        \
                        widgets/layersselect.cpp        \
                        widgets/scalewidget.cpp         \
                        widgets/rotatewidget.cpp        \
                        widgets/incutparams.cpp         \
                        widgets/informwidget.cpp        \
                        actions/insertaction.cpp

FORMS =                 ui/debug.ui                     \
                        ui/layers.ui                    \
                        ui/textpanel.ui                 \
                        ui/exportdoc.ui                 \
                        coordarea.ui                    \
                        ui/scalewidget.ui               \
                        ui/createdoc.ui                 \
                        ui/print.ui                     \
                        ui/maprubber.ui                 \
                        ui/layersselection.ui           \
                        ui/importsxf.ui                 \
                        ui/savebitmap.ui                \
                        ui/rotatesettings.ui            \
                        ui/incutsettings.ui             \
                        ui/docinfo.ui

LIBS +=                 -lmeteo.global                  \
                        -lmeteodata                     \
                        -lmeteo.font                    \
                        -lmeteo.formsh                  \
                        -lmeteo.geobasis                \
                        -lmeteo.landmask                \
                        -lmeteo.map                     \
                        -lmeteo.map.customui            \
                        -lmeteo.proto                   \
                        -lmeteo.textproto               \
                        -lmnfuncs                       \
                        -lmnmathtools                   \
                        -lmeteo.sql                      \
                        -lobanal                        \
                        -lprotobuf                      \
                        -ltapp                          \
                        -ltdebug                        \
                        -ltrpc                          \
                        -ltsingleton                    \
                        -lmeteo.mainwindow

INCLUDEPATH +=          actions                         \
                        widgets

include( $$(SRC)/include.pro )
