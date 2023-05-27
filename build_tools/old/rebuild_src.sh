#!/bin/sh
# ���������� ���� �������� � ������ rpm

ERR_SRC=66

if [ $# -lt 2 ]; then
  echo "������������� ���������: `basename $0` <��� �������> <system | debug>"
  exit -1
fi


if [ -z $SRC ]; then
  echo "�� ���������� ���������� ��������� SRC"
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
