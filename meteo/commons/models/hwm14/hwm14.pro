TEMPLATE = lib
TARGET   = meteo.hwm14

HEADERS             = funcs.h                     \

SOURCES             = funcs.cpp                   \

FORTRAN_SOURCES     = hwm14.f90                   \

LIBS               += -ltapp                      \
                      -lmnmathtools               \

HWM_FILES           = dwm07b104i.dat              \
                      hwm123114.bin               \
                      gd2qd.dat                   \

hwm.files = $$HWM_FILES
hwm.path  = $$(BUILD_DIR)/etc/meteo/models/hwm
!win32 { hwm.extra = chmod a+r $$HWM_FILES }
INSTALLS +=  hwm

include( $$(SRC)/include.pro )
include( $$(SRC)/fortran.pri )
