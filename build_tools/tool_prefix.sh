#!/bin/bash
#
# Получает информацию о пакете
#

proj=$1

if [ ! -f $SRC/$proj/projects.list ]
then
  exit 1
fi

. $SRC/$proj/projects.list
echo $PACKAGE_PREFIX
