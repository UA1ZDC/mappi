#!/bin/sh
# Получить архив с исходным кодом ПК Метрика TODO переделать для всех ПК

PWD=`pwd`

APP_NAME=$1


DSTDIR=~/tmp

if [ ! -d "$DSTDIR" ]; then
  mkdir $DSTDIR
fi

rm -rf $DSTDIR/src
mkdir $DSTDIR/src
svn export http://srv.homeunix.org/svn/src/mn_commons $DSTDIR/src/mn_commons
svn export http://srv.homeunix.org/svn/src/replaces $DSTDIR/src/replaces
svn export http://srv.homeunix.org/svn/src/arm_vgm $DSTDIR/src/arm_vgm
svn export http://srv.homeunix.org/svn/src/${APP_NAME} $DSTDIR/src/${APP_NAME}
svn export http://srv.homeunix.org/svn/src/build_tools $DSTDIR/src/build_tools
svn export http://srv.homeunix.org/svn/src/pack_lists $DSTDIR/src/pack_lists
svn export http://srv.homeunix.org/svn/src/truetype $DSTDIR/src/truetype
svn export http://srv.homeunix.org/svn/src/Makefile $DSTDIR/src/Makefile
svn export http://srv.homeunix.org/svn/src/common.mk $DSTDIR/src/common.mk
svn export http://srv.homeunix.org/svn/src/env-bash $DSTDIR/src/env-bash
svn export http://srv.homeunix.org/svn/src/include.pro $DSTDIR/src/include.pro
svn export http://srv.homeunix.org/svn/src/termcolors.mk $DSTDIR/src/termcolors.mk
cd $DSTDIR
tar -cvvzf ${APP_NAME}.src.tar.gz src
cd $PWD
rm -rf $DSTDIR/src
