TARGET = asmjit
TEMPLATE = lib

QT       -= core gui

DESTDIR = $(BUILD_DIR)/lib

SOURCES += \
    base/arch.cpp \
    base/assembler.cpp \
    base/codebuilder.cpp \
    base/codecompiler.cpp \
    base/codeemitter.cpp \
    base/codeholder.cpp \
    base/constpool.cpp \
    base/cpuinfo.cpp \
    base/func.cpp \
    base/globals.cpp \
    base/inst.cpp \
    base/logging.cpp \
    base/operand.cpp \
    base/osutils.cpp \
    base/regalloc.cpp \
    base/runtime.cpp \
    base/string.cpp \
    base/utils.cpp \
    base/vmem.cpp \
    base/zone.cpp \
    x86/x86assembler.cpp \
    x86/x86builder.cpp \
    x86/x86compiler.cpp \
    x86/x86inst.cpp \
    x86/x86instimpl.cpp \
    x86/x86internal.cpp \
    x86/x86logging.cpp \
    x86/x86operand.cpp \
    x86/x86operand_regs.cpp \
    x86/x86regalloc.cpp

HEADERS += asmjit.h \
    base/arch.h \
    base/assembler.h \
    base/codebuilder.h \
    base/codecompiler.h \
    base/codeemitter.h \
    base/codeholder.h \
    base/constpool.h \
    base/cpuinfo.h \
    base/func.h \
    base/globals.h \
    base/inst.h \
    base/logging.h \
    base/misc_p.h \
    base/operand.h \
    base/osutils.h \
    base/regalloc_p.h \
    base/runtime.h \
    base/simdtypes.h \
    base/string.h \
    base/utils.h \
    base/vmem.h \
    base/zone.h \
    x86/x86assembler.h \
    x86/x86builder.h \
    x86/x86compiler.h \
    x86/x86emitter.h \
    x86/x86globals.h \
    x86/x86inst.h \
    x86/x86instimpl_p.h \
    x86/x86internal_p.h \
    x86/x86logging_p.h \
    x86/x86misc.h \
    x86/x86operand.h \
    x86/x86regalloc_p.h \
    arm.h \
    asmjit_apibegin.h \
    asmjit_apiend.h \
    asmjit_build.h \
    base.h \
    x86.h

include( $$(SRC)/include.pro )

QMAKE_CXXFLAGS += -Wno-error=class-memaccess

