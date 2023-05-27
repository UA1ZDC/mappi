#!/bin/bash

function show_help
{
    echo "$0 - скрипт для очистки проекта от атоматически генерируемых в процессе сборки файлов."
    echo "Использование: $0 [PATH]"
    echo "Если PATH не задан, используется текущая директория."
}

CD=`pwd`

if [ -n "$1" ]; then
    CD=`realpath $1`
fi

if [ -f $CD/build.sh ]; then
    ODLCD=`pwd`
    cd $CD
    $CD/build.sh distclean
    if [ $? -ne 0 ]; then
        echo "Error: clean"
        cd $OLDCD
        exit 1
    fi
    cd $OLDCD
fi

echo "[FULLCLEAN] $CD"
find $CD -name '.o'   -type d -exec rm -rf {} \; 2>/dev/null
find $CD -name '.moc' -type d -exec rm -rf {} \; 2>/dev/null
find $CD -name '.ui'  -type d -exec rm -rf {} \; 2>/dev/null
find $CD -name '.rcc' -type d -exec rm -rf {} \; 2>/dev/null

find $CD -name '*.pb.h'  -type f -exec rm -f {} \; 2>/dev/null
find $CD -name '*.pb.cc' -type f -exec rm -f {} \; 2>/dev/null

for d in $( find $CD -name 'autotest' ); do
  if [ ! -f $d/*.pro ]; then
    echo "REMOVE empty dir: $d"
    rm -rf $d
  fi
done
