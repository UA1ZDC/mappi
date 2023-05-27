#!/bin/sh
#���������� �������(���) � ������ rpm (� ����������� ����������� ��������� ��� ����������� ��������� � �������).

ERR_ARGS=65
ERR_SRC=66

while getopts ":db" option
do
  case $option in
    d ) BUILD=debug;;
    b ) need_rebuild=y
  esac
done
shift $(($OPTIND - 1))

if [ $# -lt 1 ]; then
  echo "������������� ���������: `basename $0` [�����] <��� �������>"
  echo "-d   debug ������. ��� �����, �����-��, release"
  echo "-b   ������� ������ ���������� �������"
  exit $ERR_ARGS
fi

name=$1
BUILD=system
need_rebuild=n

if [ -z $SRC ]; then
  echo "�� ���������� ���������� ��������� SRC"
  exit $ERR_SRC
fi

cd $SRC
. ./env-bash $BUILD
cd -
echo "����������� ������ ������ � ���������� $BUILD_DIR"

#export SRC=`pwd`/.. TODO �� ������ �����. ������������

path=$SRC/build_tools/
$path/check_rpmmacros.sh 
err=$?
if [ 0 -ne $err ]; then
  exit $err
fi

if [ -e $SRC/$name/specs/install_${name}.sh ]; then
  echo "��������������� �������� ����� �������"
  . $SRC/$name/specs/install_${name}.sh
fi

if [ $need_rebuild == "y" ]; then
  echo "���������� �������" 
  $path/rebuild_src.sh $name $BUILD
  err=$?
  if [ 0 -ne $err ]; then
    echo "������ ���������� �������"
    exit $err
  fi
fi

$SRC/build_tools/specgenerator.sh $name
cd $SRC

make altpack
#������������������ ������ �� �������. ���������� ��������. (���������� ��� ������ rpm ���������� � �������, � �� � Makefile)
#run script for build rpm.
#path=$SRC/build_tools
#$path/altpack.sh $1


echo "������� ������ ������� �������"
$path/build_prj.sh $name 
err=$?
if [ 0 -ne $err ]; then
  echo "������ ������� ������ ������� �������"
  exit $err
fi

echo "����� ������� ������ � $BUILD_DIR/tar/$name"

exit 0
