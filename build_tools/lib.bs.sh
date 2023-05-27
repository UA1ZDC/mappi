#!/bin/bash

# -----------------------------------------------------------------------------
# описание:
#   Проверяет наличие элемента в массиве
# агрументы:
#   $1 - проверяемый элемент
#   $2 - массив "${arr[@]}"
# коды возврата:
#   0   - элемент отсутствует в массиве
#   1   - элемент присутствует в массиве
f_array_contains()
{
    local seeking=$1; shift
    for element; do
        if [[ $element == $seeking ]]; then
            return 1
        fi
    done
    return 0
}


# -----------------------------------------------------------------------------
# описание:
#   Исключает дубликаты из списка (массива)
function f_remove_duplicates()
{
  local arr=($(echo "${!1}" | tr ' ' '\n' | awk '!x[$0]++'))
  
  for proj in "${arr[@]}"
  do 
    echo $proj; 
  done
}


# -----------------------------------------------------------------------------
# описание:
#   Возвращает путь к проекту относительно корневого каталога с исходным кодом
# агрументы:
#   $1 - абсолютный или относительный путь к проекту
# коды возврата:
#   0   - успешное завершение
#   1   - не удалось определить корневую директорию с исходным кодом из пути к проекту
f_project_path()
{
    local sep='/'
    local path=`readlink -f $1`
    
    # заменяем '\' на '/'
    path=${path//\\//}

    # echo "исходный путь: $path"

    # разбираем путь к проекту на отдельные директории
    arr=()
    arg=$path
    while true; do
        case "$arg" in
            *"$sep"*)
                arr+=( "${arg%%$sep*}$sep" )
                arg=${arg#*$sep}
                ;;
            *)
                arr+=( "$arg" )
                break
                ;;
        esac
    done

    # ищем директорию, в которой содержитя build_tools/lib.bs.sh (это и будет директория, 
    # относительно которой будет отсчитываться путь к проекту)
    curPath=""
    for a in "${arr[@]}"
    do
        # echo "проверяем путь: $curPath"
        curPath=$curPath$a
        test -f "$curPath/build_tools/lib.bs.sh"
        res=$?
        if [[ 0 == $res ]]; then
            # echo "корень SRC: $curPath"
            proPath=${path/$curPath}
            # echo "путь к проекту: $proPath"
            echo "$proPath"
            return 0
        fi
    done

    return 1
}


# -----------------------------------------------------------------------------
# описание:
#   Составляет список зависимостей от пакетов ОС
# агрументы:
#   $1 - тип зависимостей( 0 - для разработки, 1 - для установки )
# коды возврата:
#   $2 - имя проекта
#   0   - успешное завершение
#   1   - содержит несуществующие пути
function f_mk_os_package_list()
{
  local setup_deps=$1
  local project_path=$2
  local dep_file_name=${SRC}/${project_path}/installpack/skelet/apt
  if [[ "ASTRA" == $(f_distr_name) ]]
  then
    dep_file_name="${dep_file_name}/astra"
  else 
    dep_file_name="${dep_file_name}/debian"
  fi

  if [[ 0 == $setup_deps ]]
  then
    dep_file_name="${dep_file_name}.dev.deps"
  else
    dep_file_name="${dep_file_name}.setup.deps"
  fi 
  
  if [[ -e ${dep_file_name} ]]
  then
    cat ${dep_file_name}
  else    
    exit 1
  fi
}

# -----------------------------------------------------------------------------
# описание:
#   Проверяет существование директорий.
# агрументы:
#   $1 - массив путей
# коды возврата:
#   0   - успешное завершение
#   1   - содержит несуществующие пути
function f_check_dirs()
{
    bs_G_LAST_ERROR=()
    for i in "${!1}"
    do
        if [[ -d $SRC/$i ]]
        then
            : echo "dir: $i"
        else
            bs_G_LAST_ERROR+=("Ошибка: путь '$SRC/$i' не существует")
        fi
    done

    if [[ ${#bs_G_LAST_ERROR[@]} != 0 ]]; then
        return 1
    fi
}


# -----------------------------------------------------------------------------
# агрументы:
#   $1 - путь проекта относительно $SRC
# коды возврата:
#   0   - успешное завершение
#   1   - ошибка при выполнении этапа qmake
#   2   - ошибка при выполнении этапа make
#   3   - ошибка при выполнении этапа make install
function f_qmake_make_install()
{
    local old_dir=`pwd`

    function on_exit()
    {
        cd $old_dir
    }

    cd $SRC/$i

    echo -e "${ct_green}[Q]${c_def} $i"
    qmake 1> /dev/null
    if [[ 0 != $? ]]; then
        on_exit
        return 1
    fi

    echo -e "${ct_green}[B]${c_def} $i"
    make -j`nproc` 1> /dev/null
    if [[ 0 != $? ]]; then
        on_exit
        return 2
    fi

    echo -e "${ct_mag}[I]${c_def} $i"
    make install 1> /dev/null
    if [[ 0 != $? ]]; then
        on_exit
        return 3
    fi

    on_exit
    return 0
}

# -----------------------------------------------------------------------------
# описание:
#   Выполняет цели clean и distclean make-файла (если он есть).
#   Удаляет директории (.o, .moc. .ui, .rcc и файлы *.pb.h, *.pb.cc).
# агрументы:
#   $1 - путь проекта относительно $SRC
# коды возврата:
#   0   - успешное завершение
#   1   - ошибка
function f_fullclean()
{
    local old_dir=`pwd`
    local i=$1

    function on_exit() {
        cd $old_dir
    }

    cd $SRC/$i

    echo -e "${ct_yel}[CLEAN] $i ${c_def}"
    test -f Makefile && make clean 2>&1>/dev/null
    test -f Makefile && make distclean 2>&1> /dev/null

    test -d '.o'   && ( rm -rf '.o'   2>&1>/dev/null || return 1 )
    test -d '.moc' && ( rm -rf '.moc' 2>&1>/dev/null || return 1 )
    test -d '.ui'  && ( rm -rf '.ui'  2>&1>/dev/null || return 1 )
    test -d '.rcc' && ( rm -rf '.rcc' 2>&1>/dev/null || return 1 )
    test -d '.tbs_props' && ( rm -rf '.tbs_props' 2>&1>/dev/null || return 1 )

    find . -name '*.pb.h'  -type f -exec rm -f {} \; 2>&1>/dev/null
    if [[ 0 != $? ]]; then on_exit; return 1; fi
    find . -name '*.pb.cc' -type f -exec rm -f {} \; 2>&1>/dev/null
    if [[ 0 != $? ]]; then on_exit; return 1; fi

    on_exit
    return 0
}

# -----------------------------------------------------------------------------
# описание:
#   Возвращает название дистрибутива (DEBIAN,ASTRA) или UNKNOWN если определить не удалось.
function f_distr_name
{
  local UNAME=`uname -a`

  if [[ $UNAME == *"Debian"* ]]; then
    echo "DEBIAN"
  elif [[ $UNAME == *"astra"* ]]; then
    echo "ASTRA"
  else 
    echo 'UNKNOWN'
  fi
}
