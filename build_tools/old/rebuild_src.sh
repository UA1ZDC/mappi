#!/bin/sh
# Пересборка всех проектов и сборка rpm

ERR_SRC=66

if [ $# -lt 2 ]; then
  echo "использование программы: `basename $0` <имя проекта> <system | debug>"
  exit -1
fi


if [ -z $SRC ]; then
  echo "Не определена переменная окружения SRC"
  exit $ERR_SRC
fi

old_dir=`pwd`
cd $SRC
export SRC=`pwd`
. ./env-bash $2
make clean;
rm -rf $BUILD_DIR
if [ -n "$1" ]; then 
  make -C $1 msvs install
else
  make install
fi


exit 0
