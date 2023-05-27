TEMPLATE = lib
TARGET   = meteo.map.view.drawtools
CONFIG += warn_on
QT += widgets

LOG_MODULE_NAME = vizdoc

HEADERS =   drawtoolbarwidget.h           \
            polygondrawobject.h           \
            drawtoolsaction.h             \
            panelwidget.h                 \
            propwidget.h                  \
            highlightobject.h             \
            groupdrawobject.h             \
            drawobject.h                  \
            textdrawobject.h              \
            pixmapdrawobject.h            \
            polygonpropwidget.h           \
            textposdlg.h                  \
            geoobjectitem.h               \
            grouppropwidget.h             \
            symbdlg.h                     \
            arrowdlg.h                    \
            undoredo.h                    \
            frontpropwidget.h             \
            iconsetwidget.h               \
            pixmaptoolgraphicsitem.h      \
            geographtextitem.h            \
            graphicsgroupitem.h           \
            controlpanelpixmapwidget.h    \
            geoproxywidget.h              \
            controlpaneltextwidget.h      \
            pixmappropwidget.h            \
            textpropwidget.h              \
            groupobject.h

SOURCES =   drawtoolbarwidget.cpp         \
            polygondrawobject.cpp         \
            drawtoolsaction.cpp           \
            panelwidget.cpp               \
            propwidget.cpp                \
            highlightobject.cpp           \
            groupdrawobject.cpp           \
            drawobject.cpp                \
            textdrawobject.cpp            \
            pixmapdrawobject.cpp          \
            polygonpropwidget.cpp         \
            textposdlg.cpp                \
            geoobjectitem.cpp             \
            grouppropwidget.cpp           \
            symbdlg.cpp                   \
            arrowdlg.cpp                  \
            undoredo.cpp                  \
            frontpropwidget.cpp           \
            iconsetwidget.cpp             \
            pixmaptoolgraphicsitem.cpp    \
            geographtextitem.cpp          \
            graphicsgroupitem.cpp         \
            controlpanelpixmapwidget.cpp  \
            geoproxywidget.cpp            \
            controlpaneltextwidget.cpp    \
            pixmappropwidget.cpp          \
            textpropwidget.cpp            \
            groupobject.cpp

FORMS =     drawtoolbarwidget.ui          \
            polygonpropwidget.ui          \
            textposdlg.ui                 \
            grouppropwidget.ui            \
            frontpropwidget.ui            \
            pixmapminwidget.ui            \
            textminwidget.ui              \
            controlpanelpixmapwidget.ui   \
            controlpaneltextwidget.ui

LIBS +=     -lprotobuf                    \

LIBS +=     -ltapp                        \
            -ltdebug                      \
            -lmeteo.sql

LIBS +=     -lmeteo.map.view              \
            -lmeteo.graphitems            \
            -lmeteo.map                   \
            -lmeteo.proto                 \
            -lmeteo.global                \
            -lmeteo.geobasis              \
            -lmnmathtools                 \
            -ltcustomui

include( $$(SRC)/include.pro )
