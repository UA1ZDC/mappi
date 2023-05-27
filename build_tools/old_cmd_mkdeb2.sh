#!/bin/bash --
# Скрипт создания DEB пакета

. $SRC/termcolors.mk
. $SRC/build_tools/lib.bs.sh

function package_name()
{  
  local NAME=`echo $1 | sed 's/\//-/g'`
  echo "maslo${NAME}"
}

function bs_mod_mkdeb_help()
{
    echo ""
    echo "Использование: mkdeb [опции] -- [проект|продукт,...]"
    echo "Опции:"
    echo "  -h - вывод помощи"    
    echo "  --prefix=PREFIX - устанавливает PRJ_DIR"
    echo ""
}


function bs_mod_mkdeb_short_help()
{
  echo "clean             - очистка проектов"
}

function f_qmake_mkdeb()
{  
  local old_dir=`pwd`

  function on_exit()
  {
    cd $old_dir
  }

  i=$1
  cd $SRC/$i
  local OLD_BUILD_DIR=$BUILD_DIR
  local OLD_LIBRARY_PATH=$LIBRARY_PATH    
  export LIBRARY_PATH=$LIBRARY_PATH:$BUILD_DIR/lib
  export PACKAGE_DIR=$BUILD_DIR/package/$PACKAGE_NAME
  export BUILD_DIR=$BUILD_DIR/package/$PACKAGE_NAME/$PRJ_DIR
  
  echo -e "${ct_green}[PQ]${c_def} $i"  
  qmake 1> /dev/null
  if [[ 0 != $? ]]; then
    on_exit
    return 1
  fi

  echo -e "${ct_mag}[PB]${c_def} $i"
  make -j`nproc` 1> /dev/null
  if [[ 0 != $? ]]; then
    on_exit
    return 1
  fi
  
  echo -e "${ct_mag}[PI]${c_def} $i"
  make install 1> /dev/null
  if [[ 0 != $? ]]; then
    on_exit
    return 1
  fi
  
  export LIBRARY_PATH=$OLD_LIBRARY_PATH
  
  BUILD_DIR=$OLD_BUILD_DIR  
  on_exit
  return 0
}

function f_qmake_mkdep()
{  
  local OLD_PWD=`pwd`
  cd $SRC/$1  
  bash build.sh mkdeb --prefix=$PRJ_DIR --type=$PACK_TYPE --nobuild --silent
  if [[ 0 != $? ]]; then exit 1; fi
  cd $OLD_PWD
}

function make_dpkg() {
  local PROJECT_NAME=${PWD##*/}
  local MAJOR_VERSION=1
  local MINIOR_VERSION=2
  if [ -z "$PACKAGE_REVISION" ] 
  then
    echo "NEW REVISION"
    if [ -f "${SRC}/build_tools/revision" ]
    then
      export PACKAGE_REVISION=`cat ${SRC}/build_tools/revision`
    else
      export PACKAGE_REVISION=0
    fi
    NEXT_VERSION=$((PACKAGE_REVISION+1))
    echo $NEXT_VERSION > ${SRC}/build_tools/revision
  fi
  
  local PROJECT_PATH=`pwd`
  local PROJECT_NAME=`package_name ${PROJECT_PATH#$SRC}`   
  local PACKAGE_NAME="${PROJECT_NAME}_${MAJOR_VERSION}.${MINIOR_VERSION}.${PACKAGE_REVISION}"

  local PACKAGE_PATH=${BUILD_DIR}/package/${PACKAGE_NAME}
    
  if [ -f "${BUILD_DIR}/package/$PACKAGE_NAME.deb" ]
  then   
    echo "Пакет уже собран: ${BUILD_DIR}/package/$PACKAGE_NAME.deb"
    exit 0  
  else
    echo "Сборка пакета: $PACKAGE_NAME.deb, конфигурация: $PACK_TYPE, каталог установки: $PRJ_DIR"
  fi

  
  mkdir -p $PACKAGE_PATH &> /dev/null
  mkdir $PACKAGE_PATH/DEBIAN &> /dev/null

  local projects=($(f_mk_projects_list build_PROJ[@] 1))
  local deps=($(f_mk_projects_list build_PROJ[@] -1))

  local profile=`pwd`/projects.list

  if [ ! -f "$profile" ]
  then
    echo $path
    exit 1
  fi

  . $profile      
      
  for pr in "${DEPS[@]}"
  do
    f_qmake_mkdep $pr
    if [[ 0 != $? ]]; then exit 1; fi
  
    
    local DEP_NAME=`package_name /${pr}`
    if [ -z "$DEPENDSLIST" ]
    then
      DEPENDSLIST="${DEP_NAME}(>=${MAJOR_VERSION}.${MINIOR_VERSION}.${PACKAGE_REVISION})"
    else
      DEPENDSLIST="${DEPENDSLIST}, ${DEP_NAME}(>=${MAJOR_VERSION}.${MINIOR_VERSION}.${PACKAGE_REVISION})"
    fi            
  done
    
  local DEPS_TYPE=1
  if [ $PACK_TYPE == "release" ] 
  then
    DEPS_TYPE=1
  fi
  OSDEPS=(`f_mk_os_package_list $DEPS_TYPE ${PROJECT_PATH#$SRC}`)  
  echo ${OSDEPS[@]}
  for dep in "${OSDEPS[@]}"
  do
    if [ -z "$DEPENDSLIST" ]
    then
      DEPENDSLIST="${dep}"      
    else
      DEPENDSLIST="${DEPENDSLIST}, ${dep}"      
    fi  
  done
  
  for pr in "${DIRS[@]}"
  do    
    f_qmake_mkdeb $pr
    if [[ 0 != $? ]]; then exit 1; fi
  done  
  
  if [[ "$PACK_TYPE" == "release" ]]
  then
    echo "STRIP"
    TMP_FILE="/tmp/striplisttmp.txt"
    find "$PACKAGE_PATH" -type f > $TMP_FILE

    while read LINE
    do
      strip --strip-debug  "$LINE" &> /dev/null
    done < "$TMP_FILE"
  fi


  echo "Package: ${PROJECT_NAME}" >> $PACKAGE_PATH/DEBIAN/control
  echo "Version: ${MAJOR_VERSION}.${MINIOR_VERSION}.${PACKAGE_REVISION}" >> $PACKAGE_PATH/DEBIAN/control
  echo "Section: base" >> $PACKAGE_PATH/DEBIAN/control
  echo "Priority: optional" >> $PACKAGE_PATH/DEBIAN/control
  echo "Architecture: amd64" >> $PACKAGE_PATH/DEBIAN/control
  echo "Maintainer: Ivanov Ivan Ivanovich" >> $PACKAGE_PATH/DEBIAN/control
  if [ -f $PWD/installpack/description ]
  then
    echo "Description: `cat $PWD/installpack/description`; Пакет изготовлен `date` "  >> $PACKAGE_PATH/DEBIAN/control     
  else
    
    echo "Description: Общие файлы СПО, пакет изготовлен `date` " >> $PACKAGE_PATH/DEBIAN/control
  fi
  
  if [ -z "${DEPENDSLIST}" ]
  then
    >/dev/null
  else
    echo "Depends: $DEPENDSLIST" >> $PACKAGE_PATH/DEBIAN/control
  fi    
    
  if [ -f $PWD/installpack/*.pro ]
  then    
    cd $PWD/installpack
    echo "Сборка пакета установки $PWD"
    f_qmake_mkdeb ${PROJECT_PATH#$SRC}/installpack
    if [ -f $PACKAGE_PATH/DEBIAN/preinst ]
    then
      chmod 755 $PACKAGE_PATH/DEBIAN/preinst
    fi
    if [ -f $PACKAGE_PATH/DEBIAN/postinst ]
    then
      chmod 755 $PACKAGE_PATH/DEBIAN/postinst
    fi
    if [ -f $PACKAGE_PATH/DEBIAN/prerm ]
    then
      chmod 755 $PACKAGE_PATH/DEBIAN/prerm
    fi
    if [ -f $PACKAGE_PATH/DEBIAN/postrm ]
    then
      chmod 755 $PACKAGE_PATH/DEBIAN/postrm
    fi
    cd ..
  fi
  
  find "$PACKAGE_PATH/$PRJ_DIR/etc/"  -type f -printf "$PRJ_DIR/etc/%P\n"  > $PACKAGE_PATH/DEBIAN/conffiles
  find "$PACKAGE_PATH/$PRJ_DIR/share/"  -type f -printf "$PRJ_DIR/share/%P\n"  >> $PACKAGE_PATH/DEBIAN/conffiles

  if [ -d "$PACKAGE_PATH/$PRJ_DIR/etc" ]; then
    cp -rp $PACKAGE_PATH/$PRJ_DIR/etc $PACKAGE_PATH/$PRJ_DIR/etc.default
  fi
  if [ -d "$PACKAGE_PATH/$PRJ_DIR/share" ]; then
    cp -rp $PACKAGE_PATH/$PRJ_DIR/share $PACKAGE_PATH/$PRJ_DIR/share.default
  fi
  dpkg-deb --build $PACKAGE_PATH
}

build_PROJ=()
build_OPTS=()
PACK_TYPE="release"
for i in "$@"
do
case $i in
    -h|--help)
      bs_mod_mkdeb_help
      exit 0
    ;;    
    --short-help)
     bs_mod_mkdeb_short_help
     exit 0
    ;;
    --prefix=*)
      PREFIX=${i#--prefix=}        
      shift
    ;;
    --type=*)
      PACK_TYPE=${i#--type=}
      if [[ "$PACK_TYPE" != "release" && "$PACK_TYPE" != "develop" ]]
      then
        PACK_TYPE=""
      fi
      shift
    ;;
    -nb|--nobuild)
      NOBUILD="true"
      shift
    ;;
    -s|--silent)
      SILENT="true"
      shift
    ;;
    --) 
      break
    ;;
esac
done

if [ -z $SILENT ]
then
  if [ -z $PACK_TYPE ]
  then
   while true
    do
      echo "Выберите тип устанавливаемых пакетов"
      read -p "<r>elease / <d>evelop[по умолчанию: release]: " input
      case "$input" in
        [Rr]|"" )
          PACK_TYPE="release"
          break;;
        [Dd] )
          PACK_TYPE="develop"
          break;;           
        *) ;;
      esac
    done
  fi  
  
  if [ -z "$NOBUILD" ]
  then
    while true
    do
      echo "Произвести очистку каталога сборки?"
      read -p "<Yy>: Очистить, <Nn> : не очищать: [по умолчанию: Очистить]" input
      case "$input" in
        [Nn] )          
          NOBUILD="true"
          break
        ;;          
        *)          
          break;;
      esac
    done
  fi
  
  if [ -z $PREFIX ]
  then
    while true
    do      
      read -p "Укажите директорию для установки пакетов: [по умолчанию /opt/maslo]" input
      case "$input" in      
      [\s]+|"")
        PREFIX="/opt/maslo"
        break;;
      *)
        PREFIX="$input"
        break;;
      esac
    done  
  fi
fi

PRJ_DIR=$PREFIX

if [ -z "$NOBUILD" ]
then
  bash build.sh clean -a && bash build.sh 
  if [[ 0 != $? ]]; then exit 1; fi
fi

make_dpkg
if [[ 0 != $? ]]
then 
  exit 1
fi
