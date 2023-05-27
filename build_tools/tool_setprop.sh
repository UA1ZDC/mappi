#!/bin/bash

bs_PROP_FILE=
bs_INST_DIR=
bs_ORIG_DIR=
bs_ORIG_FILES=()
bs_USE_ENV=false

bs_PARSE_STATE='wait_command'
for i in "$@"
do
case $i in
    --)
        bs_PARSE_STATE='wait_projects'
        shift
    ;;
    --prop=*)
        bs_PROP_FILE=${i#--prop=}
        shift
    ;;
    --inst=*)
        bs_INST_DIR=${i#--inst=}
        shift
    ;;
    --orig=*)
        bs_ORIG_DIR=${i#--orig=}
        shift
    ;;
    --env*)
        bs_USE_ENV=true
        shift
    ;;
    *)
        if [[ "wait_projects" == "$bs_PARSE_STATE" ]]; then
            bs_ORIG_FILES+=("${i}")
        else
            echo ""
            echo -e $ct_red"Внутренняя ошибка: неверное значение переменной bs_PARSE_STATE='${bs_PARSE_STATE}'"$c_def
            echo ""
            exit 1
        fi
        shift
    ;;
esac
done

#echo "bs_PROP_FILE: $bs_PROP_FILE"
#echo "bs_INST_DIR: $bs_INST_DIR"
#echo "bs_ORIG_DIR: $bs_ORIG_DIR"
#echo "bs_ORIG_FILES: ${bs_ORIG_FILES[@]}"

#if [[ "x" == "x$bs_PROP_FILE" ]]
#then
#    echo "Ошибка: необходимо указать файл совйств"
#    exit 1
#fi

if [[ "x" == "x$bs_INST_DIR" ]]
then
    echo "Ошибка: необходимо указать директорию с установленными файлами"
    exit 2
fi

if [[ "x" == "x$bs_ORIG_DIR" ]]
then
    echo "Ошибка: необходимо указать директорию с исходными файлами"
    exit 3
fi

#for f in "${bs_ORIG_FILES[@]}"; do echo "file: $f" done

# формируем список файлов для выполнения замены
bs_FILES=()
for f in ${bs_ORIG_FILES}
do
    absPath=$(readlink -f "${bs_ORIG_DIR}/${f}")
    for absf in ${absPath}; do
      #echo "[DEBUG] abs path: $absf"
      baseName=$(basename "$absf")
      bs_FILES+=("${baseName}")
    done
done

#for f in "${bs_FILES[@]}"; do echo "files: $f"; done

bs_FILE_PATHS=()
for f in "${bs_FILES[@]}"; 
do 
  bs_FILE_PATHS+=("${bs_INST_DIR}/${f}"); 
done


for f in "${bs_FILE_PATHS[@]}" 
do  
    # подставляем значения из файла параметров
    if [[ -f $bs_PROP_FILE ]]; then
        while IFS= read -r line
        do
            if [[ "x" == "x${line}" ]] || [[ $line = \#* ]]; then continue; fi
            
            bs_PROP_KEY="@${line%=*}@"
            bs_PROP_VALUE=${line##*=}
            
            #echo "debug: bs_PROP_KEY=$bs_PROP_KEY   bs_PROP_VALUE=$bs_PROP_VALUE"
            sed "s|${bs_PROP_KEY}|${bs_PROP_VALUE}|g" "${f}" > "${f}.new"
            chmod --reference="${f}" "${f}.new"
            mv -f ${f}.new ${f}
        done < ${bs_PROP_FILE}
    fi

    # подставляем значения переменных окружения
    if [[ "xtrue" = "x$bs_USE_ENV" ]]; then
        bs_ENVLIST=()
        IFS=$'\n' 
        for e in $(env | grep "^TBS_");   do bs_ENVLIST+=(${e}); done
        for e in $(env | grep "PRJ_DIR"); do bs_ENVLIST+=(${e}); done
	for e in $(env | grep "HOME"); do bs_ENVLIST+=(${e}); done

        for line in "${bs_ENVLIST[@]}"; do
            bs_PROP_KEY="@${line%=*}@"
            bs_PROP_VALUE=${line##*=}
            #echo "[DEBUG] line=$f   bs_PROP_KEY=$bs_PROP_KEY   bs_PROP_VALUE=$bs_PROP_VALUE"
            sed "s|${bs_PROP_KEY}|${bs_PROP_VALUE}|g" "${f}" > "${f}.new"
            chmod --reference="${f}" "${f}.new"
            mv -f ${f}.new ${f}
        done      
    fi
done



