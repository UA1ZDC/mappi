TARGET       =  thematic
TEMPLATE     =  lib
QT          +=  widgets

DESTDIR      =  $(BUILD_DIR)/lib/plugins/mappi.app.widgets

INCLUDEPATH +=  $$(SRC)/mappi/ui/exptguid

SOURCES     +=  \
    thematicsetting.cpp       \
    exprguidepaintwidget.cpp  \
    thematicsettingwidget.cpp \
    thematiceditor.cpp        \
    expreditor.cpp            \
    variableeditor.cpp        \
    channeleditor.cpp

HEADERS     +=  \
    thematicsetting.h         \
    exprguidepaintwidget.h    \
    thematicsettingwidget.h   \
    thematiceditor.h          \
    expreditor.h              \
    variableeditor.h          \
    channeleditor.h


FORMS       +=  \
    thematicsettingwidget.ui  \
    thematiceditor.ui         \
    expreditor.ui             \
    variableeditor.ui         \
    channeleditor.ui

DISTFILES   +=  thematic.json

RESOURCES    =  $$(SRC)/mappi/etc/mappi.qrc

LIBS        += -lsettings.base            \
               -lexprguid                 \



PROTOPATH += $(SRC)/meteo/commons/proto
include( $$(SRC)/protobuf.pri )
             
include( $$(SRC)/include.pro )
