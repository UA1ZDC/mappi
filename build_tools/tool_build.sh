#!/bin/bash
# Сборка проекта
# USAGE: tool_build.sh $PROJECT
#

. $SRC/termcolors.mk
. $SRC/build_tools/lib.log.sh

PROJ=$1
. $SRC/$PROJ/projects.list



# -----------------------------------------------------------------------------
# агрументы:
#   $1 - путь проекта относительно $SRC
# коды возврата:
#   0   - успешное завершение
#   1   - ошибка при выполнении этапа qmake
#   2   - ошибка при выполнении этапа make
#   3   - ошибка при выполнении этапа make install
function f_qmake_make_install()
{
  local old_dir=`pwd`
  local i=$1

  function on_exit()
  {
    cd $old_dir
  }

  cd $SRC/$i

  echo -e "${ct_green}[Q]${c_def} $i"
  qmake 1> /dev/null
  if [[ 0 != $? ]]; then
      on_exit
      return 1
  fi

  echo -e "${ct_green}[B]${c_def} $i"
  make -j`nproc` 1> /dev/null
  if [[ 0 != $? ]]; then
      on_exit
      return 2
  fi

  echo -e "${ct_mag}[I]${c_def} $i"
  make install 1> /dev/null
  if [[ 0 != $? ]]; then
    on_exit
    return 3
  fi

  on_exit
  return 0
}

if [ ! -z "${DIRS_EXTRA[@]}" ]
then
  for BUILD in "${DIRS_EXTRA[@]}"
  do
    if ! f_qmake_make_install $BUILD
    then
      exit -1
    fi
  done
fi

for BUILD in "${DIRS[@]}"
do  
  if ! f_qmake_make_install $BUILD
  then
    exit -1
  fi
done
