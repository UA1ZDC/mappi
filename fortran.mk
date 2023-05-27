#! /usr/bin/make 

SHELL = /bin/sh 

include ${SRC}/termcolors.mk

GCCVERSION=$(shell gcc --version | grep ^gcc | sed 's/^.*3\.3\.6.*/3\.3\.6/g')

ifeq ("$(GCCVERSION)", "3.3.6")
  LFLAGS_FORT = -lg2c -shared -Wl -g
  CFLAGS_FORT = -c -fPIC -g
  G77_FORT = g77
  LIBS_FORT =
else

  LFLAGS_FORT = -shared -g
  CFLAGS_FORT = -c -fPIC -g
  G77_FORT = gfortran
  LIBS_FORT = -lgfortran
endif
