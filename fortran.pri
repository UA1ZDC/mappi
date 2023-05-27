FORT_COMPILER = gfortran
CFLAGS_FORT   = -c -fPIC -g
#LFLAGS_FORT = -shared -g

LIBS += -lgfortran

fortran_obj.name = fortran objects
fortran_obj.input = FORTRAN_SOURCES
fortran_obj.output = $$OBJECTS_DIR/${QMAKE_FILE_BASE}.o
fortran_obj.depends = ${QMAKE_FILE_BASE}${QMAKE_FILE_EXT}
fortran_obj.commands = "$${FORT_COMPILER}" "$${CFLAGS_FORT}" ${QMAKE_FILE_IN_PATH}/${QMAKE_FILE_NAME} -J${OBJECTS_DIR} -o $$OBJECTS_DIR/${QMAKE_FILE_BASE}.o
QMAKE_EXTRA_COMPILERS += fortran_obj
