#!/bin/bash

function f_mod_clean_help()
{
    echo ""
    echo "Использование: clean [опции] -- [проект|продукт,...]"
    echo "Опции:"
    echo "  -a, --all   - очистка проектов вместе с зависимостями"
    echo ""
}

function f_mod_clean_short_help()
{
    echo "clean             - очистка проектов"
}

# Выводит сообщение об ошибке
function f_print_last_error()
{
    for i in "${bs_G_LAST_ERROR[@]}"
    do
        echo -e $ct_red$i$c_def
    done
}

. $SRC/termcolors.mk
. $SRC/build_tools/lib.bs.sh

clean_PROJ=()
clean_OPTS=()

recursive='false'

for i in "$@"
do
  case $i in
    -h|--help)
      f_mod_clean_help
      exit 0
    ;;
    --short-help)
      f_mod_clean_short_help
      exit 0    
    ;;
    --all|-a)
      recursive='true'
    ;;
    --*|-*)
      # echo 'option:' $i
      clean_OPTS+=($i)
    ;;
    *)
      # echo 'project:' $i
      clean_PROJ+=($i)
    ;;
  esac
done

export PACKAGE_PREFIX=`$SRC/build_tools/tool_prefix.sh $build_PROJ`

args="--extra --extra-deb"
if [ 'true' == $recursive ]
then
  args="$args --recursive"
fi

for PRJ in "${clean_PROJ[*]}"
do
  projects=( `$SRC/build_tools/tool_mkprojlist.sh $args $PRJ` )
  projects=( $(f_remove_duplicates projects[@]) )
  for i in "${projects[@]}"
  do
    $SRC/build_tools/tool_clean.sh $i || exit $EXIT_FAILTURE
    
  done

done


