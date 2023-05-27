#!/bin/sh

. ${SRC}/termcolors.mk

function init_packlist()
{
  if [ -z "${MAINPROJECT}" ]; then
    PACKLIST=${BUILD_DIR}/pack_lists/${PROJECT}_list
    if [ -f ${PACKLIST} ]; then
      rm ${PACKLIST}
    fi
  else
    PACKLIST=${BUILD_DIR}/pack_lists/${MAINPROJECT}_list
    return
  fi

  if [ -d ${BUILD_DIR}/SPECS ]; then
    rm -rf ${BUILD_DIR}/SPECS/*
  else
    mkdir ${BUILD_DIR}/SPECS
  fi

  if [ ! -d ${BUILD_DIR}/pack_lists ]; then
    mkdir ${BUILD_DIR}/pack_lists
  fi
}

function analyse_dependes()
{
  if [ -n "${MAINPROJECT}" ]; then
    return
  fi
  OLD_DIR=`pwd`
  cd ${TARGET_DIR}
  DEPENDES=`make print-dependes`
  for d in ${DEPENDES}
  do
    d=${d##*/}
    cd ${SRC}/build_tools
    ./specgenerator.sh ${d} ${PROJECT}
    cd ${TARGET_DIR}
    #echo $d
  done
  cd ${OLD_DIR}
}

function generate_pre_post_instructions()
{
  ls -1 ${TARGET_DIR}/specs/prepost* > /dev/null 2>&1
  if [ "$?" != "0" ]; then
    return
  fi

  for i in ${TARGET_DIR}/specs/prepost*
  do
    PREFIX="";
    SUB=${i##*/}
    SUB=${SUB#prepost.}
    if [ "prepost" = "$SUB" ]; then
      DESCRIPTION="Дополнительные инструкции проекта"
      SPECFILE=${BUILD_DIR}/SPECS/${PROJECT}prepost.spec
      cat spec.template | sed s/@PROJECT@/${PROJECT}/ | sed s/@DESCRIPTION@/"${DESCRIPTION}"/ | sed s/@TARGET@/"prepost"/ | sed s/%files// > ${SPECFILE}
      echo "${PROJECT}prepost-0.1-1.@ARCH@.rpm" >> ${PACKLIST}
    else
      DESCRIPTION="Дополнительные инструкции подпроекта ${SUB} для проекта"
      SPECFILE=${BUILD_DIR}/SPECS/${SUB}prepost.spec
      cat spec.template | sed s/@PROJECT@/${PROJECT}/ | sed s/@DESCRIPTION@/"${DESCRIPTION}"/ | sed s/@TARGET@/"${SUB}prepost"/ | sed s/%files// > ${SPECFILE}
      echo "${PROJECT}${SUB}prepost-0.1-1.@ARCH@.rpm ${SUB}" >> ${PACKLIST}
    fi
    while read line
    do
      case $line in
        '[PRE]')
          echo "%pre" >> ${SPECFILE}
          PREFIX='PRE'
          ;;
        '[POST]')
          echo "" >> ${SPECFILE}
          echo "%post" >> ${SPECFILE}
          PREFIX='POST'
          ;;
        *)
          if [ -n "${PREFIX}" ]; then
            echo ${line} >> ${SPECFILE}
          fi
          ;;
      esac
    done < $i
  done
}

function generate_spec_other()
{
  if [ -f ${TARGET_DIR}/specs/addsteps ]
  then
    mkdir -p ${BUILD_DIR}/tar/${PROJECT}
    cp ${TARGET_DIR}/specs/addsteps ${BUILD_DIR}/tar/${PROJECT}
  fi
  if [ -f ${TARGET_DIR}/specs/postinstall ]
  then
    mkdir -p ${BUILD_DIR}/tar/${PROJECT}
    cp ${TARGET_DIR}/specs/postinstall ${BUILD_DIR}/tar/${PROJECT}
  fi
  ls -1 ${TARGET_DIR}/specs/files* > /dev/null 2>&1
  if [ "$?" != "0" ]; then
    return
  fi

  for i in ${TARGET_DIR}/specs/files*
  do
    PREFIX="";
    SUB=${i##*/}
    SUB=${SUB#files.}
    if [ "files" = "$SUB" ]; then
      DESCRIPTION="Дополнительные файлы проекта"
      SPECFILE=${BUILD_DIR}/SPECS/${PROJECT}_files.spec
#      echo ololo=$i SPECFILE=$SPECFILE
      cat spec.template | sed s/@PROJECT@/${PROJECT}/ | sed s/@DESCRIPTION@/"${DESCRIPTION}"/ | sed s/@TARGET@/"files"/ > ${SPECFILE}
      echo "${PROJECT}files-0.1-1.@ARCH@.rpm" >> ${PACKLIST}
    else
      DESCRIPTION="Дополнительные файлы подпроекта ${SUB} для проекта"
      SPECFILE=${BUILD_DIR}/SPECS/${SUB}files.spec
#      echo ololo2=$i SPECFILE=$SPECFILE
      cat spec.template | sed s/@PROJECT@/${PROJECT}/ | sed s/@DESCRIPTION@/"${DESCRIPTION}"/ | sed s/@TARGET@/"${SUB}files"/ > ${SPECFILE}
      echo "${PROJECT}${SUB}files-0.1-1.@ARCH@.rpm ${SUB}" >> ${PACKLIST}
    fi

    while read line
    do
      case $line in
        '[SERVICES]')
          PREFIX='%attr(0755, root, root) /etc/init.d/'
          ;;
        '[SETTINGS]')
          PREFIX='%attr(0644,root,root)%{_build_prefix}/etc/'
          ;;
        '[SHARE]')
          PREFIX='%attr(0644,root,root)%{_build_prefix}/share/'
          ;;
        '[VAR]')
          PREFIX='%attr(0644,root,root)%{_build_prefix}/var/'
          ;;
        '[LIB]')
          PREFIX='%attr(0755,root,root)%{_build_prefix}/lib/'
          ;;
        '[FILES]')
          PREFIX='%attr(0644,root,root)%{_build_prefix}/'
          ;;
        '[DIRS]')
          PREFIX="DIRS"
          ;;
        '[PRE]')
          echo "%pre" >> ${SPECFILE}
          PREFIX='PRE'
          ;;
        '[POST]')
          echo "" >> ${SPECFILE}
          echo "%post" >> ${SPECFILE}
          PREFIX='POST'
          ;;
#        *)
#          if [ -n "${PREFIX}" ]; then
#            echo ${line} >> ${SPECFILE}
#          fi
#          ;;
        *)
          if [ -n "${PREFIX}" ]; then
            if [ -n "${line}" ]; then
              if [ "PRE" = "${PREFIX}" ] || [ "POST" = "${PREFIX}" ]; then
                echo ${line} >> ${SPECFILE}
              elif [ "DIRS" = "${PREFIX}" ]; then
                mkdir -p ${BUILD_DIR}/${line}
                lp='%dir%attr(0755,root,root)%{_build_prefix}'
                echo ${lp}/${line} >> ${SPECFILE}
              else
                echo ${PREFIX}${line} >> ${SPECFILE}
              fi
            fi
          fi
          ;;
      esac
    done < $i
  done
}

function add_extra_target()
{
  SUBPROJECT=`echo $1 | tr -d ' ' | tr -d '\t' | sed 's/SUBPROJECT=//'`
  SPECFILE=${BUILD_DIR}/SPECS/${SUBPROJECT}.spec
  if [ -f ${SPECFILE} ]; then
    tmp=`cat ${SPECFILE} | grep "$2"`
    if [ 0 -ne $? ]; then
      echo $2 >> ${SPECFILE}
    fi
  else
    echo "${PROJECT}${SUBPROJECT}-0.1-1.@ARCH@.rpm ${SUBPROJECT}" >> ${PACKLIST}

    echo -e "${ctb_yel}Создан spec-файл подпроекта ${SUBPROJECT}.spec ${c_def}"
    DESCRIPTION="Подпроект ${SUBPROJECT} проекта"
    cd ${OLD_DIR}
    cat spec.template | sed s/@PROJECT@/$PROJECT/ | sed s/@DESCRIPTION@/"$DESCRIPTION"/ | sed s/@TARGET@/"${SUBPROJECT}"/ > ${SPECFILE}
    echo $2 >> ${SPECFILE}
    cd ${TARGET_DIR}
  fi
}

function extract_targets()
{
  OLD_DIR=`pwd`
  cd $TARGET_DIR
  SUBD=`make print-targets`
  LFILES[0]="%defattr(-,root,root)"
  BFILES[0]="%defattr(-,root,root)"
  SFILES[0]="%defattr(-,root,root)"
  for i in $SUBD
  do
    $SRC/build_tools/target_qt_test.sh $i
    if [ $? != 0 ]
    then 
      continue;
    fi

    FILENAME=`find ./$i -type f -name  *.pro | grep -v 'test' | grep -v '\.ui' | grep -v '\.o' | grep -v '\.moc' | grep -v designerplugin`
    for EACHFILE in ${FILENAME}
    do
      TARGET=`cat ${EACHFILE} | grep TARGET`
      TEMPLATE=`cat ${EACHFILE} | grep TEMPLATE | tr -d ' ' | tr -d '\t' | sed 's~TEMPLATE=~~' | sed 's~TEMPLATE\ =~~' | sed 's~TEMPLATE\t=~~'`
      if [ "" = "${TARGET}" ]; then
        TARGET=${EACHFILE%%.pro}
        TARGET=${TARGET##./*/}
      fi
##     echo $TARGET
      if [ "$TEMPLATE" = "lib" ]; then
        SUBPROJECT=`cat ${EACHFILE} | grep SUBPROJECT`
        if [ 0 -eq $? ]; then
          FILE=`echo $TARGET | tr -d ' ' | tr -d '\t' | sed 's~TARGET=~~'`
          FILE="%attr(0755, root, root) %{_build_prefix}/lib/lib${FILE}.so*"
          add_extra_target "$SUBPROJECT" "$FILE"
        else
          CNT=${#LFILES[@]}
          LFILES[$CNT]=`echo $TARGET | tr -d ' ' | tr -d '\t' | sed 's~TARGET=~~'`
          LFILES[$CNT]="%attr(0755, root, root) %{_build_prefix}/lib/lib${LFILES[$CNT]}.so*"
        fi
      elif [ "$TEMPLATE" = "appsys" ]; then
        SUBPROJECT=`cat ${EACHFILE} | grep SUBPROJECT`
        if [ 0 -eq $? ]; then
          FILE=`echo $TARGET | tr -d ' ' | tr -d '\t' | sed 's~TARGET=~~'`
          FILE="%attr(0755, root, root) %{_build_prefix}/sbin/${FILE}"
          add_extra_target "$SUBPROJECT" "$FILE"
        else
          CNT=${#SFILES[@]}
          SFILES[$CNT]=`echo $TARGET | tr -d ' ' | tr -d '\t' | sed 's~TARGET=~~'`
          SFILES[$CNT]="%attr(0755, root, root) %{_build_prefix}/sbin/${SFILES[$CNT]}"
        fi
      elif [ "$TEMPLATE" = "apptest" ]; then
	  continue
      elif [ "$TEMPLATE" = "app" ]; then
        SUBPROJECT=`cat ${EACHFILE} | grep SUBPROJECT`
        if [ 0 -eq $? ]; then
          FILE=`echo $TARGET | tr -d ' ' | tr -d '\t' | sed 's~TARGET=~~'`
          FILE="%attr(0755, root, root) %{_build_prefix}/bin/${FILE}"
          add_extra_target "$SUBPROJECT" "$FILE"
        else
          CNT=${#BFILES[@]}
          BFILES[$CNT]=`echo $TARGET | tr -d ' ' | tr -d '\t' | sed 's~TARGET=~~'`
          BFILES[$CNT]="%attr(0755, root, root) %{_build_prefix}/bin/${BFILES[$CNT]}"
        fi
      else
        echo unk = $TARGET #неясно, что делать
        echo $TEMPLATE
      fi
    done
  done
  cd $OLD_DIR
}

function generate_spec()
{
  SPECTYPE=$1

  case $SPECTYPE in
    'lib')
      DESCRIPTION="Бибилиотеки программного комплекса"
      POSTFIX="libs"
      FILES=( "${LFILES[@]}" )
      COUNT=${#LFILES[@]}
      ;;
    'app')
      DESCRIPTION="Приложения программного комплекса"
      POSTFIX="apps"
      FILES=( "${BFILES[@]}" )
      COUNT=${#BFILES[@]}
      ;;
    'sys')
      DESCRIPTION="Системные приложения программного комплекса"
      POSTFIX="sys"
      FILES=( "${SFILES[@]}" )
      COUNT=${#SFILES[@]}
      ;;
  esac

  if [ 1 = $COUNT ]; then
    return
  fi

  cat spec.template | sed s/@PROJECT@/$PROJECT/ | sed s/@DESCRIPTION@/"$DESCRIPTION"/ | sed s/@TARGET@/"$SPECTYPE"/ > ${BUILD_DIR}/SPECS/${PROJECT}${POSTFIX}.spec
  for (( i=0; i<$COUNT; ++i ))
  do
    echo "${FILES[$i]}" >> ${BUILD_DIR}/SPECS/${PROJECT}${POSTFIX}.spec
  done

  echo "" >> ${BUILD_DIR}/SPECS/${PROJECT}${POSTFIX}.spec
  echo "%pre" >> ${BUILD_DIR}/SPECS/${PROJECT}${POSTFIX}.spec
  echo "mkdir -p $PRJ_DIR/var/" >> ${BUILD_DIR}/SPECS/${PROJECT}${POSTFIX}.spec
  echo "" >> ${BUILD_DIR}/SPECS/${PROJECT}${POSTFIX}.spec
  echo "%post" >> ${BUILD_DIR}/SPECS/${PROJECT}${POSTFIX}.spec
  echo "ldconfig" >> ${BUILD_DIR}/SPECS/${PROJECT}${POSTFIX}.spec


  echo "${PROJECT}${SPECTYPE}-0.1-1.@ARCH@.rpm" >> ${PACKLIST}
}

if [ 1 -gt $# ]; then
  echo "Enter pack name:"
  while read PROJECT ; do
    if [ "" != "$PROJECT" ]; then
      break
    else
      echo "Enter pack name:"
    fi
  done
else
  PROJECT=$1
fi

if [ 1 -lt $# ]; then
  MAINPROJECT=$2
fi


TARGET_DIR=~/src.git/${PROJECT}
LFILES=""
BFILES=""
SFILES=""

cd $SRC/build_tools
echo -e "${cf_red}Проект ${PROJECT}${c_def}"
echo -e "${ctb_yel}Инициализация файла 'список пакетов'${c_def}"
init_packlist
echo -e "${ctb_yel}"Анализ зависимостей проекта"${c_def}"
analyse_dependes
echo -e "${ctb_yel}Поиск целей${c_def}"
extract_targets
echo -e "${ct_green}Поиск целей завершен${c_def}"
echo -e "${ctb_yel}Генерация spec-файла ${PROJECT}files.spec${c_def}"
generate_spec_other
echo -e "${ctb_yel}Генерация spec-файла ${PROJECT}libs.spec${c_def}"
generate_spec 'lib'
echo -e "${ctb_yel}Генерация spec-файла ${PROJECT}apps.spec${c_def}"
generate_spec 'app'
echo -e "${ctb_yel}Генерация spec-файла ${PROJECT}sys.spec${c_def}"
generate_spec 'sys'
#echo -e "${ctb_yel}Генерация spec-файла ${PROJECT}prepost.spec${c_def}"
#generate_pre_post_instructions

#удаление повторных строк
cat $PACKLIST | awk '!($0 in a) {a[$0];print}' > pack.tmp
mv pack.tmp $PACKLIST

echo -e "${ct_green}Завершено${c_def}"
