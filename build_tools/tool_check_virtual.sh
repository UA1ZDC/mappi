#!/bin/bash 

PROJ=$1

. $SRC/$PROJ/projects.list

if [ -z "$VIRTUAL" ]
then
  exit 0
else
  exit 1
fi

