#!/bin/bash

. $SRC/termcolors.mk
. $SRC/build_tools/lib.log.sh
. $SRC/build_tools/lib.bs.sh
EXIT_SUCCESS=0
EXIT_FAILTURE=1

PROJ=$1

PRJ_LIST=$SRC/$PROJ/projects.list

if [ ! -f $PRJ_LIST ]
then
  f_error_log "Не найдет файл $PRJ_LIST, очистка прервана"
  exit $EXIT_FAILTURE
fi

. $PRJ_LIST


for DIR in ${DIRS[*]}
do
  f_fullclean $DIR || exit $EXIT_FAILTURE
done

if [ ! -z "${DIRS_EXTRA[*]}" ]
then
  for DIR in "${DIRS_EXTRA[*]}"
  do
    f_fullclean $DIR || exit $EXIT_FAILTURE
  done
fi

if [ ! -z "${DIRS_EXTRA_PKG[*]}" ]
then
  for DIR in "${DIRS_EXTRA_PKG[*]}"
  do
    f_fullclean $DIR || exit $EXIT_FAILTURE
  done
fi


if [ -d $SRC/$PROJ/installpack ]
then
  if [ -f $SRC/$PROJ/installpack/*pro ]
  then
    f_fullclean $PROJ/installpack/ || exit $EXIT_FAILTURE
  fi
fi
