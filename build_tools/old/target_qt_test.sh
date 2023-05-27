#!/bin/sh
# Проверка версии QT

QT_FILE_NAME=QT_VERSION

CURPATH=`pwd`"/"$1
FILE=${CURPATH}/${QT_FILE_NAME}

if [ -f "$FILE" ]
then
  if [ "$QT_VERSION" != "`cat $FILE`" ]
  then
    exit 1
  fi
else
  exit 0
fi

exit 0
