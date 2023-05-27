#! /usr/bin/make

SHELL = /bin/sh

TARGET_QT_TEST = $(SRC)/build_tools/target_qt_test.sh

include ${SRC}/termcolors.mk

# ifneq ($(WIN_BUILD),)
#   export QMAKE = qmake -spec $(SRC)/build_tools/win/win32-g++ -win32
# else
#   export QMAKE = qmake
# endif

.PHONY: build $(DEPENDS_PRJ)	\
	${HEADERS_FILES}	\
	${SUBDIRS}		\
	${SUBDIRS_NONQMAKE}	\
	${COMMON_FILES}		\
        ${WRAPPED_SUBDIRS}	\
	${SUBDIRS_MSVS}		\
	install-depends		\
	nodeps			\
	msvs			\
	qmake-clean		\
	nonqmake-clean		\
	wrapped-clean		\
	msvs-clean		\
	qmake-uninstall		\
	nonqmake-uninstall	\
	msvs-uninstall		\
	print-targets		\
	print-dependes

#	translate-depends	\
#	translate-clean translate-uninstall
#	$(SUBDIRS_TRANSLATE)


# ***** BUILD *****
all: build 

build: build-deps			\
       ${HEADERS_FILES}			\
       ${SUBDIRS_NONQMAKE}		\
       ${SUBDIRS}			\
       $(WRAPPED_SUBDIRS)

rebuild: clean-all uninstall-all remove-build build install
#rebuild-tr: translate-clean translate-uninstall rebuild translate

build-deps:
	@for i in ${DEPENDS_PRJ};     \
	do	                      \
	  if [[ -z "$$dd" ]]; then dd="$(DONE_DEPS)"; fi;\
	  echo -e ${ct_mag}"BUILD DEPENDS: $$i"${c_def};\
	  if [[ *"$(DONE_DEPS)"* =~ "$$i" && -n "$(DONE_DEPS)" ]]; then\
            echo -e ${ct_blue}"ALREADY DONE: $$i"${c_def};\
	  else ${MAKE} -sC $$i DONE_DEPS=$$dd; fi;\
	  if [ $$? -ne 0 ]; then break; fi; \
	  echo -e ${ct_yel}"BUILD DEPENDS: $$i -- OK"${c_def};\
	  dd="$$dd""$$i";\
	done

msvs:	msvs-deps		\
	${SUBDIRS_MSVS}

msvs-deps:
	@for i in ${DEPENDS_PRJ};     \
	do	                      \
	  if [[ -z "$$dd" ]]; then dd="$(DONE_DEPS)"; fi;\
	  echo -e ${ct_mag}"BUILD MSVS DEPENDS: $$i"${c_def};\
	  if [[ *"$(DONE_DEPS)"* =~ "$$i" && -n "$(DONE_DEPS)" ]]; then\
            echo -e ${ct_blue}"ALREADY DONE: $$i"${c_def};\
	  else ${MAKE} -sC $$i msvs DONE_DEPS=$$dd; fi;\
	  if [ $$? -ne 0 ]; then break; fi; \
	  echo -e ${ct_yel}"BUILD MSVS DEPENDS: $$i -- OK"${c_def};\
	  dd="$$dd""$$i";\
	done


install-depends:
	@for i in ${DEPENDS_PRJ};     \
	do	                      \
	  if [[ -z "$$dd" ]]; then dd="$(DONE_DEPS)"; fi;\
	  echo -e ${ct_mag}"INSTALL DEPENDS: $$i"${c_def};\
	  if [[ *"$(DONE_DEPS)"* =~ "$$i" && -n "$(DONE_DEPS)" ]]; then\
            echo -e ${ct_blue}"ALREADY INSTALLED: $$i"${c_def};\
	  else ${MAKE} -sC $$i install DONE_DEPS=$$dd; fi;\
	  if [ $$? -ne 0 ]; then break; fi; \
	  echo -e ${ct_yel}"INSTALLED DEPENDS: $$i -- OK"${c_def};\
	  dd="$$dd""$$i";\
	done

#translate-depends:
#	@for i in ${DEPENDS_PRJ};	\
#	do				\
#	  echo -e ${ct_mag}"TRANSLATE DEPENDS: $$i"${c_def};\
#	  ${MAKE} -sC $$i translate;	\
#	  if [ $$? -ne 0 ]; then break; fi; \
#	  echo -e ${ct_yel}"TRANSLATED DEPENDS: $$i -- OK"${c_def};\
#	done


${SUBDIRS}:
	@echo -e ${ct_green}"[Q] $@"${c_def}
	@for i in $@/*.pro;			\
	do					\
	  $(TARGET_QT_TEST) $@;			\
	  if [ $$? = 0 ];			\
	  then					\
	    ${QMAKE} $$i -o `pwd`/$@/Makefile;	\
	    echo -e ${ct_yel}"[B]"${c_def}"$@";	\
	    ${MAKE} -sC $@;			\
	    echo -e ${ct_blue}"[I]"${c_def}"$@";\
	    ${MAKE} -sC $@ install;		\
	  fi;					\
	done

#${SUBDIRS_TRANSLATE}:
#	@echo -e ${ct_green}"[Q] $@"${c_def}
#	@cd $@ && ${QMAKE}
#	@echo -e ${ct_blue}"[I]"${c_def}"$@"
#	@${MAKE} -sC $@ install

${SUBDIRS_NONQMAKE}:
	@echo -e ${ct_green}"[B] $@"${c_def}
	@$(MAKE) -sC $@
	@echo -e ${ct_blue}"[I]"${c_def}"$@"
	@$(MAKE) -sC $@ install

${SUBDIRS_MSVS}:
	@echo -e ${ct_blue}"[I]"${c_def}"$@"
	@$(MAKE) -sC $@ install

${COMMON_FILES}:
	@echo -e ${ct_green}"[B] $@"${c_def}
	@$(MAKE) -sC $@

${HEADERS_FILES}:
	@echo -e ${ct_green}"[B] $@"${c_def}
	@$(MAKE) -sC $@ install

${WRAPPED_SUBDIRS}:
	@echo -e ${ct_green}"[B] $@"${c_def}
	@$(MAKE) -sC $@

#install-all: install translate
install-all: install

install: install-depends	\
	${COMMON_FILES}		\
        ${HEADERS_FILES}	\
	${SUBDIRS_NONQMAKE}	\
	${SUBDIRS}		\
	$(WRAPPED_SUBDIRS)

#translate: translate-depends	\
#	 ${SUBDIRS_TRANSLATE}


nodeps:	${COMMON_FILES}		\
	${HEADERS_FILES}	\
	${SUBDIRS_NONQMAKE}     \
	${SUBDIRS}              \
	$(WRAPPED_SUBDIRS)

print-targets:
	@echo $(SUBDIRS)

print-dependes:
	@echo $(DEPENDS_PRJ)
	@for i in ${DEPENDS_PRJ}; \
	do \
	  cd $$i; \
	  make -s print-dependes; \
	  cd ${OLD_PWD}; \
	done

# ***** CLEAN *****

clean: qmake-clean nonqmake-clean wrapped-clean headers-uninstall

clean-all: deps-clean-all clean

clean-msvs: deps-msvs-clean msvs-clean

qmake-clean:
	@dir=`pwd`; \
	for i in ${SUBDIRS}; \
	do \
	  $(TARGET_QT_TEST) $$i;		\
	  if [ $$? = 0 ];			\
	  then					\
	    cd $$i;				\
	    echo "[Q] $$i";			\
	    ${QMAKE} *.pro;			\
	    echo "[X]" $$i;			\
	    ${MAKE} -s clean;			\
	    cd $$dir;				\
	    rm -f $$i/Makefile;			\
	  fi;					\
	done

#translate-clean:
#	@set -e; \
#	dir=`pwd`; \
#	for i in ${SUBDIRS_TRANSLATE}; \
#	do \
#	  cd $$i;  \
#	  echo "[Q] $$i"; \
#	  ${QMAKE}; \
#	  cd $$dir; \
#	  echo "[C] $$i"; \
#	  ${MAKE} -sC $$i clean; \
#	done


nonqmake-clean:
	@for i in ${SUBDIRS_NONQMAKE};		\
	do					\
	  echo "[X]" $$i;			\
	  ${MAKE} -sC $$i clean;		\
	done

msvs-clean:
	@for i in ${SUBDIRS_MSVS};		\
	do					\
	  echo "[X]" $$i;			\
	  ${MAKE} -sC $$i uninstall;		\
	done

wrapped-clean:
	@dir=`pwd`; \
	for i in ${WRAPPED_SUBDIRS}; \
	do \
	  echo "[X]" $$i; \
	  ${MAKE} -sC $$i clean;  \
	done

deps-clean:
	@for i in ${DEPENDS_PRJ};	\
	do	                        \
	  echo "CLEAN DEPENDS $$i";     \
	  ${MAKE} -sC $$i clean;	\
	done

deps-msvs-clean:
	@for i in ${DEPENDS_PRJ};	\
	do	                        \
	  echo "CLEAN MSVS DEPENDS $$i";\
	  ${MAKE} -sC $$i clean-msvs;	\
	done

deps-clean-all:
	@for i in ${DEPENDS_PRJ};	\
	do	                        \
	echo "CLEAN-ALL DEPENDS $$i";   \
	  ${MAKE} -sC $$i clean-all;	\
	done


# ***** UNINSTALL *****

uninstall: qmake-uninstall nonqmake-uninstall wrapped-clean common-uninstall headers-uninstall
uninstall-all: deps-uninstall-all uninstall

uninstall-msvs: deps-msvs-clean msvs-clean

qmake-uninstall:
	@set -e; \
	dir=`pwd`; \
	for i in ${SUBDIRS}; \
	do \
	  $(TARGET_QT_TEST) $$i;		\
	  if [ $$? = 0 ];			\
	  then					\
	    cd $$i;				\
	    ${QMAKE} *.pro;			\
	    cd $$dir;				\
	    ${MAKE} -sC $$i distclean uninstall;\
	  fi;					\
	done

#translate-uninstall:
#	@set -e; \
#	dir=`pwd`; \
#	for i in ${SUBDIRS_TRANSLATE}; \
#	do \
#	  cd $$i;  \
#	  echo "[Q] $$i"; \
#	  ${QMAKE}; \
#	  cd $$dir; \
#	  echo "[C] $$i"; \
#	  ${MAKE} -sC $$i clean; \
#	  echo "[X] $$i"; \
#	  rm $$i/Makefile; \
#	done

nonqmake-uninstall:
	@set -e; \
	dir=`pwd`; \
	for i in ${SUBDIRS_NONQMAKE}; \
	do \
	  ${MAKE} -sC $$i uninstall; \
	done

common-uninstall:
	@dir=`pwd`; \
	for i in ${COMMON_FILES}; \
	do \
	  echo "[X]" $$i; \
	  ${MAKE} -siC $$i uninstall; \
	done

headers-uninstall:
	@dir=`pwd`; \
	for i in ${HEADERS_FILES}; \
	do \
	  echo "[X]" $$i; \
	  ${MAKE} -sC $$i uninstall; \
	done

deps-uninstall:
	@for i in ${DEPENDS_PRJ};	\
	do	                        \
	  echo "UNINSTALL DEPENDS $$i"; \
	  ${MAKE} -sC $$i uninstall;	\
	done

deps-uninstall-all:
	@for i in ${DEPENDS_PRJ};	\
	do	                        \
	  echo "UNINSTALL-ALL DEPENDS $$i"; \
	  ${MAKE} -sC $$i uninstall-all;\
	done

# ***** REMOVE *****

remove-build:
	@echo "REMOVE-BUILD ${BUILD_DIR}"; \
	rm -rf ${BUILD_DIR}
