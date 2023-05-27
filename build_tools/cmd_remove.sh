#!/bin/bash

function f_mod_remove_help()
{
    echo ""
    echo "Использование: remove"
    echo ""
}

function f_mod_remove_short_help()
{
    echo "remove            - удаление директории сборки"
}

. $SRC/termcolors.mk

for i in "$@"
do
case $i in
    -h|--help)
        f_mod_remove_help
        exit 0
    ;;
    --short-help)
        f_mod_remove_short_help
        exit 0
    ;;
esac
done

rm -rf $BUILD_DIR
if [[ 0 != $? ]]
then
    echo -e "${ct_red}Не удалось удалить директорию '$BUILD_DIR' ${c_def}"
    exit 1
else
    echo -e "${ct_yel}[REMOVE] Директория '$BUILD_DIR' успешно удалена ${c_def}"
fi











