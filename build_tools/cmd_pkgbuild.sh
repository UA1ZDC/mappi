#!/bin/bash -x

. $SRC/termcolors.mk
. $SRC/build_tools/lib.bs.sh
. $SRC/build_tools/lib.log.sh


function bs_mod_pkgbuild_short_help()
{
  echo "pkgbuild          - сборка deb-пакетов"
}

function bs_mod_pkgbuild_help()
{
    echo ""
    echo "Использование: build pkgbuild"
    echo "Опции:"        
    echo "      --prefix=PREFIX - устанавливает PRJ_DIR"    
    echo ""
}


function f_mk_package_build_dir()
{
  PACKAGE_DIR=$1
  PROJ=$2
  PROJECT_NAME=`f_mk_package_name $PROJ`
  PACKAGE_BUILD_DIR="$PACKAGE_DIR/$PROJECT_NAME"
  echo $PACKAGE_BUILD_DIR
}

function f_mk_pkg_deps()
{
  for PROJ in $*
  do
    echo $PROJ
    cat $SRC/$PROJ/projects.list    
    . $SRC/$PROJ/projects.list    
    echo ${DEPS[*]}
    f_mk_pkg_deps ${DEPS[*]}
  done
}


build_PROJ=()
build_OPTS=()

RELEASE=YES
INCVERSION=0

for i in "$@"
do
case $i in
    -h|--help)
        bs_mod_pkgbuild_help
        exit 0
    ;;
    --short-help)
        bs_mod_pkgbuild_short_help
        exit 0
    ;;    
    --prefix=*)
        export PRJ_DIR=${i#--prefix=}        
        shift
    ;;
    --debug|-d)
      RELEASE=NO
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

if [ ! -f $SRC/build_tools/revision ]
then 
  echo "MAJOR_VERSION=1" > $SRC/build_tools/revision
  echo "MINIOR_VERSION=0" >> $SRC/build_tools/revision
  echo "REVISION=0" >> $SRC/build_tools/revision  
fi

. $SRC/build_tools/revision
if [[ -z "$MAJOR_VERSION" || -z "$MINIOR_VERSION" || -z "$REVISION" ]]
then
  echo "Неверное содержимое файла revision. Переменные MAJOR_VERSION, MINIOR_VERSION, REVISION должны быть обязательно определены"
  exit 1
fi

export VERSION="$MAJOR_VERSION.$MINIOR_VERSION.$REVISION"

echo "${build_OPTS[@]}"
if ! f_array_contains "--inc-revision" "${build_OPTS[@]}"
then
  REVISION=$((REVISION+1))
  echo "MAJOR_VERSION=$MAJOR_VERSION" > $SRC/build_tools/revision
  echo "MINIOR_VERSION=$MINIOR_VERSION" >> $SRC/build_tools/revision
  echo "REVISION=$REVISION" >> $SRC/build_tools/revision  
fi

export PACKAGE_PREFIX=`$SRC/build_tools/tool_prefix.sh $build_PROJ`
  
if [ "$PRJ_DIR" == "$BUILD_DIR" ]
then
  if [ -z "$PACKAGE_PREFIX" ]
  then
    f_error_log "Требуется задать либо PACKAGE_PREFIX"
    exit 1
  else
    export PRJ_DIR="/opt/$PACKAGE_PREFIX"
  fi
fi

echo "Сборка пакетов от" `date` "версия пакетов" $MAJOR_VERSION.$MINIOR_VERSION.$REVISION
echo "Параметы сборки: "
echo "    PRJ_DIR --> ${PRJ_DIR}"
echo "  BUILD_DIR --> ${BUILD_DIR}"
echo "    RELEASE --> ${RELEASE}"
echo "    PRJ_DIR --> ${PRJ_DIR}"

f_progress_log "Сборка отдельных пакетов"

PRJ_LIST=()
for PRJ in ${build_PROJ[*]}
do
  PRJ_LIST+=( `$SRC/build_tools/tool_mkprojlist.sh --recursive --extra-deb $PRJ ` )
done
PRJ_LIST=( `f_remove_duplicates PRJ_LIST[@]` )

#Эти переменные используются дальше, в скрипте tool_pkgbuild.sh
export PRJ_DIR=$PRJ_DIR
export BUILD_DIR=$BUILD_DIR
export VERSION=$VERSION
export RELEASE=$RELEASE

export REPO_DIR=$BUILD_DIR/$PACKAGE_PREFIX

if [ -d "$REPO_DIR" ]
then
  rm -r $REPO_DIR
fi

for REPO in contrib main non-free
do
  if ! mkdir -p $REPO_DIR/pool/$REPO
  then
    f_error_log "Ошибка при создании каталога для собранных пакетов $REPO_DIR/pool/main"
    exit 1
  fi
done

#Сборка пакетов в репозиторий main
for PROJ in ${PRJ_LIST[*]}
do  
  if ! $SRC/build_tools/tool_pkgbuild.sh $PROJ
  then
    f_error_log "Ошибка при сборке пакета $PROJ"
    exit 1
  fi
done

#Импорт пакетов non-free и contrib
if [ ! -z $DEB_NON_FREE_PATH ]
then
  cp $DEB_NON_FREE_PATH/*.deb $REPO_DIR/pool/non-free
fi

if [ ! -z $DEB_CONTRIB_PATH ]
then
  cp $DEB_CONTRIB_PATH/*.deb $REPO_DIR/pool/contrib
fi

for APTTOOL in aptftp.conf privatekey.gpg  publickey.gpg  update.sh
do
  if ! cp $SRC/build_tools/apt/$APTTOOL $REPO_DIR/
  then
    f_error_log "Ошибка при копировании файла $APTTOOL"
  fi
done
