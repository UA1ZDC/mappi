#!/bin/bash

function bs_mod_build_help()
{
    echo ""
    echo "Использование: build [опции] -- [проект|продукт,...]"
    echo "Опции:"
    echo "  -D, --nodeps        - сборка проектов без зависимостей"
    echo "      --conf=CONF     - сборка с параметрами конфигурации CONF"
    echo "      --prefix=PREFIX - устанавливает PRJ_DIR"
    echo ""
}

function bs_mod_build_short_help()
{
    echo "build             - сборка проектов"
}

. $SRC/termcolors.mk
. $SRC/build_tools/lib.bs.sh
EXIT_FAILTURE=1
EXIT_SUCCESS=0

# Выводит сообщение об ошибке
function f_print_last_error()
{
    for i in "${bs_G_LAST_ERROR[@]}"
    do
        echo -e $ct_red$i$c_def
    done
}

build_PROJ=()
build_OPTS=()
skipDeps='false'
for i in "$@"
do
case $i in
    -h|--help)
        bs_mod_build_help
        exit 0
    ;;
    --short-help)
        bs_mod_build_short_help
        exit 0
    ;;
    --conf=*)
        export PROP_NAME=${i#--conf=}
        shift
    ;;
    --prefix=*)
        export PRJ_DIR=${i#--prefix=}
        echo $PRJ_DIR
        shift
    ;;
    -D|--nodeps)
        export NODEPS=TRUE
        shift
    ;;
    --*|-*)
        # echo 'option:' $i
        build_OPTS+=($i)
        shift
    ;;
    *)
        # echo 'project:' $i
        build_PROJ+=($i)
        shift
    ;;
esac
done


PRJ_LIST=()
for PRJ in ${build_PROJ[*]}
do
  if [[ "TRUE" == "$NODEPS" ]]
  then
    PRJ_LIST+=$PRJ
  else
    PRJ_LIST+=( `$SRC/build_tools/tool_mkprojlist.sh --recursive --extra $PRJ ` )
  fi
done
PRJ_LIST=( `f_remove_duplicates PRJ_LIST[@]` )

for PROJ in "${PRJ_LIST[@]}"
do  
  if ! $SRC/build_tools/tool_build.sh $PROJ
  then
    exit $EXIT_FAILTURE
  fi
done

exit $EXIT_SUCCESS
##############################################################################################################################################################################################################################################



skip_deps=0
f_array_contains "-D" "${build_OPTS[@]}"
if [[ 1 == $? ]]; then skip_deps=1; fi
f_array_contains "--no-deps" "${build_OPTS[@]}"
if [[ 1 == $? ]]; then skip_deps=1; fi
buildDeps=1
projects=($(f_mk_projects_list build_PROJ[@] $skip_deps $buildDeps ))
projects=($(f_remove_duplicates projects[@]))


build_tests=0
f_array_contains "-T" "${build_OPTS[@]}"
if [[ 1 == $? ]]; then build_tests=1; fi
f_array_contains "--test" "${build_OPTS[@]}"
if [[ 1 == $? ]]; then build_tests=1; fi

if [[ 1 == $build_tests ]];
then
    for prj in ${projects[@]};
    do
        if [[ -d $SRC/$prj/test/ ]]; then
            if [[ 1 == `ls $SRC/$prj/test/ | grep ".pro" -c` ]]; then
                projects+=($prj/test/)
            fi
        fi
    done
fi


build_autotests=0
f_array_contains "-A" "${build_OPTS[@]}"
if [[ 1 == $? ]]; then build_autotests=1; fi
f_array_contains "--autotest" "${build_OPTS[@]}"
if [[ 1 == $? ]]; then build_autotests=1; fi

if [[ 1 == $build_autotests ]];
then
    for prj in ${projects[@]};
    do
        if [[ -d $SRC/$prj/autotest/ ]]; then
            if [[ 1 == `ls $SRC/$prj/autotest/ | grep ".pro" -c` ]]; then
                projects+=($prj/autotest/)
            fi
        fi
    done
fi


# проверяем наличие всех проектов
f_check_dirs projects[@]
if [[ 0 != $? ]]; then
    f_print_last_error
    exit 1
fi

for i in "${projects[@]}"
do
    # echo -e "${ct_green}${i}${c_def}"

    f_qmake_make_install $i
    if [[ 0 != $? ]]; then exit 1; fi
done









