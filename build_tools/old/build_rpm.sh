#!/bin/sh
#сборка rpm пакета

if [ $# -ne 1 ]; then
  echo "использование программы: `basename $0` <имя пакета>"
  exit 65
fi

#имя spec-файла пакета
pack=$1

mkdir -p $BUILD_DIR/RPMS
str=`rpmbuild -bb $pack 2>&1`
err=$?
if [ $err -ne 0 ]; then
  echo "$str"
fi

exit $err
