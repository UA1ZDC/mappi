TEMPLATE  = lib

TARGET    = meteo.document.view
QT       += widgets printsupport

LOG_MODULE_NAME = vizdoc


LIBS +=       -ltdebug              \
              -ltapp                \
              -ltrpc                \
              -lprotobuf            \
              -lmeteo.nosql         \
              -ltcustomui           \
              -lmeteo.sql

LIBS +=       -lmeteo.etc           \
              -lmeteo.proto         \
              -lmeteo.global        \
              -lmeteo.settings      \
              -lmeteo.etc           \
              -lmeteo.geobasis      \
              -lmeteo.map           \
              -lmeteo.map.view      \
              -lpoppler-qt5         \
              -lmeteo.ui.condition  \
              -lmeteo.viewheader    \
              -lmeteo.prewiewwidget \
              -lmeteo.map.dataexchange

include( $$(SRC)/include.pro )

HEADERS += \
    wgtdocview/wgtdocview.h \    
    datamodel/docviewtypes.h \
    datamodel/dataservice.h \    
    custom/filesystemselectionwidget.h \
    datamodel/wgtdocviewModel.h \
    custom/wgtdocviewpanel.h \
    faxgeoobject.h \
    markerflagitem.h \
    faxaction.h \
    custom/pindialog.h \
    datamodel/pinningstorage.h \
    custom/wgthoveringwidget.h \
    custom/imagedisplaygraphicsitem.h \
    custom/animationgraphicsitem.h \
    custom/animationcontroller.h \
    custom/ctrlclickbutton.h \
    pluginhandler.h

SOURCES += \
    wgtdocview/wgtdocview.cpp \     
    datamodel/docviewtypes.cpp \
    datamodel/dataservice.cpp \    
    custom/filesystemselectionwidget.cpp \
    datamodel/wgtdocviewModel.cpp \
    custom/wgtdocviewpanel.cpp \
    faxgeoobject.cpp \
    markerflagitem.cpp \
    faxaction.cpp \
    custom/pindialog.cpp \
    datamodel/pinningstorage.cpp \
    custom/wgthoveringwidget.cpp \
    custom/imagedisplaygraphicsitem.cpp \
    custom/animationgraphicsitem.cpp \
    custom/animationcontroller.cpp \
    custom/ctrlclickbutton.cpp \
    pluginhandler.cpp

FORMS += \
    wgtdocview/wgtdocview.ui \    
    custom/filesystemselectionwidget.ui \
    custom/wgtdocviewpanel.ui \
    custom/pindialog.ui \
    custom/wgthoveringwidget.ui
