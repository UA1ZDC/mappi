#!/bin/bash

function bs_mod_check_build_help()
{
    echo ""
    echo "Использование: checkbuild [опции] [проект|продукт,...]"
    echo "Опции:"
    echo "  -T, --check-test     - включает проверку сборки тестов"
    echo "  -A, --check-autotest - включает проверку сборки автотестов"
    echo ""
}

function bs_mod_checkbuild_short_help()
{
    echo "checkbuild        - проверка сборки"
}

. $SRC/termcolors.mk
. $SRC/build_tools/lib.bs.sh

# Выводит сообщение об ошибке
function f_print_last_error()
{
    for i in "${bs_G_LAST_ERROR[@]}"
    do
        echo -e $ct_red$i$c_def
    done
}

t_PROJ=()
t_OPTS=()
for i in "$@"
do
case $i in
    -h|--help)
        bs_mod_checkbuild_help
        exit 0
    ;;
    --short-help)
        bs_mod_checkbuild_short_help
        exit 0
    ;;
    --*|-*)
        # echo 'option:' $i
        t_OPTS+=($i)
        shift
    ;;
    *)
        # echo 'project:' $i
        t_PROJ+=($i)
        shift
    ;;
esac
done

build_OPTS=''

f_array_contains "--check-test" "${t_OPTS[@]}"
if [[ 1 == $? ]]; then build_OPTS="$build_OPTS --test"; fi

f_array_contains "--check-autotest" "${t_OPTS[@]}"
if [[ 1 == $? ]]; then build_OPTS="$build_OPTS --autotest"; fi


/bin/bash $SRC/build_tools/tbs remove || exit $?
/bin/bash $SRC/build_tools/tbs clean --all -- $t_PROJ || exit $?
/bin/bash $SRC/build_tools/tbs build $build_OPTS -- $t_PROJ || exit $?









