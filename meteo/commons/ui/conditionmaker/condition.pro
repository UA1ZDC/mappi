TEMPLATE = lib
TARGET   = meteo.ui.condition

QT      += widgets

LIBS                 += -lmeteo.global

include( $$(SRC)/include.pro )

HEADERS += \
    conditions/conditionlogical.h \
    conditions/conditionstring.h \
    conditions/conditionint.h \
    conditions/conditionnull.h \
    conditions/conditionoid.h \
    templates/conditiongenerator.h \
    conditiondialogs/conditioncreationdialogint.h \    
    conditiondialogs/conditionaldialogstring.h \
    conditions/conditiondateinterval.h \
    conditions/conditiondouble.h \
    conditiondialogs/conditiondialogdouble.h \
    conditions/conditionlongint.h \
    conditiondialogs/conditiondialoglongint.h \
    condition_widgets/longintvalidator.h \
    conditiongeneric.h \
    templates/conditiongenericprivate.h \
    conditions/conditionboolean.h \
    conditions/conditionstringlist.h

SOURCES += \
    conditions/conditionstring.cpp \
    conditions/conditionint.cpp \
    conditions/conditionnull.cpp \
    conditions/conditionoid.cpp \
    conditiondialogs/conditioncreationdialogint.cpp \    
    conditiondialogs/conditionaldialogstring.cpp \
    conditions/conditiondateinterval.cpp \
    conditions/conditiondouble.cpp \
    conditiondialogs/conditiondialogdouble.cpp \
    conditions/conditionlongint.cpp \
    conditiondialogs/conditiondialoglongint.cpp \
    condition_widgets/longintvalidator.cpp \
    conditiongeneric.cpp \
    conditions/conditionlogical.cpp \
    templates/conditiongenericprivate.cpp \
    conditions/conditionboolean.cpp \
    conditions/conditionstringlist.cpp

FORMS += \
    condition_widgets/conditionwidgetint.ui \
    condition_widgets/conditionwidgetstring.ui \
    condition_widgets/conditionwidgetdouble.ui \
    condition_widgets/conditionwidgetlongint.ui

