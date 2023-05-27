#!/bin/bash

path=$1
ignore=$2


if [[ "x" == "x$path" ]]; then
  echo "Ошибка: не указан путь к проекту"
  exit 1
fi

profile=$SRC/$path/projects.list
test -f "$profile"
res=$?
if [[ 0 == $res ]]; then  
  . $profile
  
  if [[ "1" != "$ignore" ]]
  then    
    for pr in "${DEPS[@]}"
    do
      $0 $pr $ignore
      echo $pr
    done
    echo $path
  fi
  if [[ "-1" != "$ignore" ]]
  then 
    for pr in "${DIRS[@]}"
    do
      echo $pr
    done
  fi
fi
