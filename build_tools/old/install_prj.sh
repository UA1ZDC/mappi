#!/bin/sh
#устанавливает все пакеты, указанные в файле

ERR_ARGS=65

UNINSTALL="uninstall-sh"

pack=@PACK@

dir=`dirname $0`

if [ $# -eq 1 ]; then
  prj=$1
else
  prj=$dir/${pack}_list
fi

if [ ! -f $prj ];then
  #echo "ERROR: не найден файл со списком пакетов" >> $LOG_FILE
  exit $ERR_ARGS
fi


un_file=uninstall_${pack}-sh
cp $dir/$UNINSTALL /usr/local/sbin/$un_file
#cp $prj /usr/local/sbin/

while read line ; do
  words=( $line )
  if [ ${#words[@]} -ge 2 ]
  then
    cnt=${#parameters[@]}
    found=0
    for (( i=0; i<$cnt; ++i ))
    do
      if [ ${words[1]} = ${parameters[$i]} ]; then
        found=1
        break
      fi
    done
    if [ 0 -eq ${found} ];then
      parameters[${#parameters[@]}]=${words[1]}
    fi
  fi
done < $prj

echo "Выберите параметр установки [$(echo ${parameters[@]} | tr " " "|" )]:"

read option

if [ -f $dir/addsteps ]
then
  . $dir/addsteps
fi

while read line ; do
  words=( $line )
  if [[ ${#words[@]} -ge 2 && "${words[1]}" != "$option" ]]
  then
    echo "пакет ${words[0]} пропущен в соответствии с параметром установки $option"
  else
#    echo $dir/${words[0]}
    rpm -Uvh --replacefiles --nodeps $dir/${words[0]}
  fi
done < $prj

if [ -f $dir/postinstall ]
then 
  . $dir/postinstall $option
fi

exit 0

