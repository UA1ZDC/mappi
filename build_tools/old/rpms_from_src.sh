#!/bin/sh
#Пересборка проекта(тов) и сборка rpm (с правильными переменными окружения для последующей установки в систему).

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
  echo "использование программы: `basename $0` [опции] <имя проекта>"
  echo "-d   debug сборка. без опции, соотв-но, release"
  echo "-b   сделать полную пересборку проекта"
  exit $ERR_ARGS
fi

name=$1
BUILD=system
need_rebuild=n

if [ -z $SRC ]; then
  echo "Не определена переменная окружения SRC"
  exit $ERR_SRC
fi

cd $SRC
. ./env-bash $BUILD
cd -
echo "Выполняется сборка релиза в директории $BUILD_DIR"

#export SRC=`pwd`/.. TODO не поняла зачем. закомментила

path=$SRC/build_tools/
$path/check_rpmmacros.sh 
err=$?
if [ 0 -ne $err ]; then
  exit $err
fi

if [ -e $SRC/$name/specs/install_${name}.sh ]; then
  echo "Предварительный сценарий сбоки пакетов"
  . $SRC/$name/specs/install_${name}.sh
fi

if [ $need_rebuild == "y" ]; then
  echo "Пересборка пакетов" 
  $path/rebuild_src.sh $name $BUILD
  err=$?
  if [ 0 -ne $err ]; then
    echo "Ошибка пересборки пакетов"
    exit $err
  fi
fi

$SRC/build_tools/specgenerator.sh $name
cd $SRC

make altpack
#закомментированный способ не доделан. необходимо додумать. (инструкции для сборки rpm желательно в скрипте, а не в Makefile)
#run script for build rpm.
#path=$SRC/build_tools
#$path/altpack.sh $1


echo "Содание набора пакетов проекта"
$path/build_prj.sh $name 
err=$?
if [ 0 -ne $err ]; then
  echo "Ошибка содания набора пакетов проекта"
  exit $err
fi

echo "Набор пакетов собран в $BUILD_DIR/tar/$name"

exit 0
