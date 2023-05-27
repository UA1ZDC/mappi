#!/bin/bash 

. $SRC/termcolors.mk
. $SRC/build_tools/lib.bs.sh
. $SRC/build_tools/lib.log.sh

function f_mk_package_name()
{  
  local NAME=`echo $1 | sed 's/\//-/g'`
  echo "${PACKAGE_PREFIX}-${NAME}"
}

function f_qmake_make_install()
{  
  BUILD=$1

  local old_dir=`pwd`

  cd $SRC/$BUILD
  
  echo -e "${ct_green}[PQ]${c_def} $BUILD"  
  qmake 1> /dev/null
  if [[ 0 != $? ]]; then
    cd $old_dir
    return 1
  fi

  echo -e "${ct_mag}[PB]${c_def} $BUILD"
  make -j`nproc` 1> /dev/null
  if [[ 0 != $? ]]; then
    cd $old_dir
    return 1
  fi
  
  echo -e "${ct_mag}[PI]${c_def} $BUILD"
  make install 1> /dev/null
  if [[ 0 != $? ]]; then
    cd $old_dir
    return 1
  fi
  
  cd $old_dir
  return 0
}

function f_arr_join
{
  local IFS="$1";
  shift; 
  echo "$*";
}

PROJ=$1
PROVIDES=$2

. $SRC/$PROJ/projects.list

if [ ! -z "$VIRTUAL" ]
then
  for PACKAGE in ${VIRTUAL[*]}
  do
    if ! $SRC/build_tools/tool_pkgbuild.sh $PACKAGE $PROJ
    then
      f_error_log "Ошибка при сборке виртуального пакета $PACKAGE"
      exit 1
    fi
  done
  exit 0
fi

PACKAGE_NAME=`f_mk_package_name $PROJ`

OLD_BUILD_DIR=$BUILD_DIR

export LIBRARY_PATH=$LIBRARY_PATH:$BUILD_DIR/lib
export PACKAGE_DIR=$BUILD_DIR/packages/$PACKAGE_NAME/
export BUILD_DIR=$BUILD_DIR/packages/$PACKAGE_NAME/$PRJ_DIR

f_command_log "Сборка $PROJ, версии $VERSION, с очистикой отладочных симовлов: $RELEASE"

PROJECT_NAME=`f_mk_package_name $PROJ`
  
if [ ! -d "$BUILD_DIR" ]
then
  mkdir -p "$BUILD_DIR"
fi

CONTROLFILE="$PACKAGE_DIR/DEBIAN/control"
DESCRIPTION_PATH=$SRC/$PROJ/installpack/description
  
mkdir -p "$PACKAGE_DIR/DEBIAN"
  
echo "Package: ${PACKAGE_NAME}" > $CONTROLFILE
echo "Version: ${VERSION}" >> $CONTROLFILE
echo "Section: base" >> $CONTROLFILE
echo "Priority: optional" >> $CONTROLFILE
echo "Architecture: amd64" >> $CONTROLFILE
echo "Maintainer: nobody@nowhere" >> $CONTROLFILE

if [ ! -z "$PROVIDES" ]
then
  PROVIDES_NAME=`f_mk_package_name $PROVIDES`
  echo "Provides: $PROVIDES_NAME" >> $CONTROLFILE
  echo "Breaks: $PROVIDES_NAME" >> $CONTROLFILE
fi
 
if [ -f  "${DESCRIPTION_PATH}" ]
then
  echo "Description: `cat ${DESCRIPTION_PATH}`; Пакет изготовлен `date` "  >> $PACKAGE_DIR/DEBIAN/control     
else
  echo "Description: СПО Маслобойка. Пакет изготовлен `date` "  >> $PACKAGE_DIR/DEBIAN/control     
fi

DEPENDS=()

for DEP in ${DEPS[*]}
do  
  DEPNAME=`f_mk_package_name $DEP`
  if $SRC/build_tools/tool_check_virtual.sh $DEP
  then
    FULLDEP="$DEPNAME(>=${VERSION})"
  else
    FULLDEP="$DEPNAME"
  fi
  DEPENDS+=(${FULLDEP})
done

DEPENDS+=(`f_mk_os_package_list 1 $PROJ`)  
echo "Depends: `f_arr_join ',' ${DEPENDS[*]}`"  >> $PACKAGE_DIR/DEBIAN/control
echo >> $PACKAGE_DIR/DEBIAN/control
 
for BUILD in ${DIRS[*]}
do  
  if ! f_qmake_make_install $BUILD
  then
    f_error_log 'Ошибка при сборке проекта.'
    exit 1
  fi
done

if [ ! -z "${DIRS_EXTRA_PKG[*]}" ]
then
  for BUILD in "${DIRS_EXTRA_PKG[*]}"
  do
    if ! f_qmake_make_install $BUILD
    then
      f_error_log 'Ошибка при сборке проекта.'
      exit 1
    fi
  done
fi
  
if [ -f $SRC/$PROJ/installpack/*.pro ]
then        
  echo "Сборка пакета установки $PWD"
  if ! f_qmake_make_install $PROJ/installpack/
  then
    f_error_log 'Ошибка при сборке installpack-а'
    exit 1
  fi
  for FILE in $PACKAGE_DIR/DEBIAN/preinst $PACKAGE_DIR/DEBIAN/postinst $PACKAGE_DIR/DEBIAN/prerm $PACKAGE_DIR/DEBIAN/postrm
  do
    if [ -f $FILE ]
    then
      chmod 755 $FILE
    fi
  done
fi

if [[ "YES" == "$RELEASE" ]]
then
  TMP_FILE=`mktemp`
  find "$PACKAGE_DIR" -type f > $TMP_FILE
  while read LINE
  do
    strip --strip-debug  "$LINE" &> /dev/null
    done < "$TMP_FILE"
  rm $TMP_FILE  
fi

if [ -d "$PACKAGE_DIR/$PRJ_DIR/etc/" ]
then
  find "$PACKAGE_DIR/$PRJ_DIR/etc/"  -type f -printf "$PRJ_DIR/etc/%P\n"  > $PACKAGE_DIR/DEBIAN/conffiles
fi

dpkg-deb --build $PACKAGE_DIR ${REPO_DIR}/pool/main/${PACKAGE_NAME}_${VERSION}.deb
