#!/bin/bash

. $SRC/termcolors.mk
. $SRC/build_tools/lib.log.sh

build_PROJ=""
recursive="false"
args=""
INCLUDE_DEPS_BUILD="false"
INCLUDE_DEPS_PKG="false"
INCLUDE_VIRTUAL_PKG="false"

function help()
{
  echo "tool_mkdeps - получение списка зависимостей"
  echo "Использование: $0 [-h|--help] [-r|--recursive] project"
  echo "Опции"
  echo "-h|--help       : Показать эту справку и выйти"
  echo "--recursive  : Построить список проектов с учетом вложенных проектов"  
  echo "--extra      : Добавить проекты только для сборки"
  echo "--extra-deb  : Добавить проекты только для сборки пакетов"  
  echo "--virtual    : Добавить проекты, помеченные как виртуальные пакеты"
}

for i in "$@"
do
  case $i in
    -h|--help)
        help
        exit 0
    ;;
    --recursive)
      args="$args $i"
      recursive="true"
    ;;    
    --extra)
      INCLUDE_DEPS_BUILD="true"
      args="$args $i"
    ;;
    --extra-deb)      
      INCLUDE_DEPS_PKG="true"
      args="$args $i"
    ;;
    --virtual)
      INCLUDE_VIRTUAL_PKG="true"
      args="$args $i"
    ;;
    *)
      build_PROJ=($i)
      break
    ;;
  esac
done

profile=$SRC/$build_PROJ/projects.list

if [[ ! -f "$profile" ]]
then  
  f_error_log "Файл $profile не обнаружен"
  exit 1
fi

. $profile

function f_mk_proj_settings()
{
  local proj=$1
  if [ -z "$proj" ]
  then
    return
  fi
  
  if [ "true" == $recursive ]
  then
    if ! $0 $args $proj 
    then
      exit 1
    fi
  fi
  echo $proj
}


for proj in "${DEPS[@]}"
do
  f_mk_proj_settings $proj
done

if [ "true" == "$INCLUDE_DEPS_BUILD" ]
then
  for proj in "${DEPS_EXTRA[@]}"
  do
    f_mk_proj_settings $proj
  done
fi

if [ "true" == "$INCLUDE_DEPS_PKG" ]
then  
  for proj in "${DEPS_EXTRA_PKG[@]}"
  do
    f_mk_proj_settings $proj
  done
fi

if [ "true" == "$INCLUDE_VIRTUAL_PKG" ]
then  
  for proj in "${VIRTUAL[@]}"
  do
    f_mk_proj_settings $proj
  done
fi
  
echo $build_PROJ
