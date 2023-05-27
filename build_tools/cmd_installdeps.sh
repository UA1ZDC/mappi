#!/bin/bash

function f_mod_installdeps_help()
{
    echo "installdeps - установка пакетов из репозитория"
    echo "Использование: installdeps [опции]"
    echo "Опции:"
    echo "  -s, --setup   установка минимального набора пакетов необходимых для запуска ПО"    
}

function bs_mod_build_short_help()
{
    echo "installdeps       - установка требуемых зависимостей для разработчика"
}

# Выводит сообщение об ошибке
function f_print_last_error()
{
    for i in "${bs_G_LAST_ERROR[@]}"
    do
        echo -e $ct_red$i$c_def
    done
}

#
# MAIN
#

. $SRC/build_tools/lib.colors.sh
. $SRC/build_tools/lib.bs.sh
. $SRC/build_tools/lib.log.sh

installDev=0
build_PROJ=()

for i in "$@"
do
case $i in
  -h|--help)
    bs_mod_installdeps_help
    exit 0
  ;;  
  --short-help)
    bs_mod_build_short_help
    exit 0
  ;;
  -s|--setup)
    installDev=1
    shift
  ;;        
  *)
    # echo 'project:' $i
    build_PROJ+=($i)
    shift
  ;;
esac
done  
  
#projects=($(tool_mkprojlist.sh f_mk_spo_deps_list ${build_PROJ[@]} ))
projects=()
for PRJ in ${build_PROJ[*]}
do
  echo $PRJ
  projects+=( `$SRC/build_tools/tool_mkprojlist.sh --recursive --extra $PRJ ` )
done
projects=( `f_remove_duplicates projects[@]` )


f_command_log "Получение списка зависимостей"
deps=()
for project in ${projects[@]}
do  
  newdeps=(`f_mk_os_package_list $installDev $project`)  
  if [ ${#newdeps[@]} -eq 0 ]
  then
    f_fail_status_log "Зависимости не найдены" "$project"
  else
    f_ok_status_log   " Найдены  зависимости " $project
    deps+=( ${newdeps[@]} )
  fi  
done

if [ ${#deps[@]} -eq 0 ]
then
  echo "Зависимости не найдены."
  exit 0
fi

f_command_log "\nПроверка состояния зависимостей"
BROKEN_DEPENDS=''
installed=($(dpkg-query -l | grep '^ii ' | cut -d' ' -f 3 | cut -d':' -f 1))
for d in ${deps[@]}
do
  f_array_contains $d ${installed[@]}
  if [[ "0" == "$?" ]]
  then
    f_fail_status_log "   Ошибка   " $d
    BROKEN_DEPENDS="$BROKEN_DEPENDS $d"
  else
    f_ok_status_log " Установлен " $d
  fi
done

if [[ -z "$BROKEN_DEPENDS" ]]
then
  exit 0
fi
  
echo -e "\n"
f_command_log "Установка пакетов"
sudo apt-get --yes install $BROKEN_DEPENDS
exit $?
