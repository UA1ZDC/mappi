#!/bin/bash

PACKAGE_ROOT=`pwd`/dist
MASLO_ROOT=$PACKAGE_ROOT/maslo
SETUP_ROOT=$PACKAGE_ROOT/setup

MAJOR_VERSION=`ls $MASLO_ROOT | grep maslo-commons | awk -F[_.] '{print $2}'`
MINIOR_VERSION=`ls $MASLO_ROOT | grep maslo-commons | awk -F[_.] '{print $3}'`
REVISION=`ls $MASLO_ROOT | grep maslo-commons | awk -F[_.] '{print $4}'`



if [[ $EUID -ne 0 ]]; then
   echo "Для установки СПО требуются права администратора" 
   exit 1

fi

echo "Выберите СПО для установки"
echo "1. СПО Банк"
echo "2. База данных для СПО Банк"
echo "3. СПО Сбор"
echo "4. СПО Прогноз"
echo "5. СПО Метеопродукция"
echo "6. СПО Погода"
echo "7. СПО Взаимодействие"

read -p "[1-7]" input

SYSTEM_PKG_LIST=(libsnappy1 libpam0g libqt5script5 libpng12-0 libpoppler-qt5-1 qt5-image-formats-plugins libtiff5 libpq5 dialog libreoffice-writer libreoffice-calc)
SETUP_LIST=(maslo_protobuf-3.4.1.deb maslo_openjpeg-2.3.1.deb)
INSTALLLIST=( maslo-cross-commons maslo-commons maslo-sql maslo-commons maslo-meteo-commons )

case "$input" in
  [1] )
    echo "СПО БАНК"
    SYSTEM_PKG_LIST+=(pkg-config libqt5core5a)
    INSTALLLIST+=(maslo-meteo-bank)
    ;;
  [2] ) 
    echo "База данных для СПО Банк"
    INSTALLLIST=(maslo-meteo-bankdb)
    SYSTEM_PKG_LIST=()
    SETUP_LIST=()    
    SETUP_LIST+=(mongodb-org-mongos_3.6.5_amd64.deb)
    SETUP_LIST+=(mongodb-org-server_3.6.5_amd64.deb)
    SETUP_LIST+=(mongodb-org-shell_3.6.5_amd64.deb)
    SETUP_LIST+=(mongodb-org-tools_3.6.5_amd64.deb)
    SETUP_LIST+=(mongodb-org_3.6.5_amd64.deb)
    ;;
  [3] )
    echo "СПО Сбор"
    INSTALLLIST+=(maslo-meteo-sbor)
    ;;
  [4] )
    echo "СПО Прогноз"
    SYSTEM_PKG_LIST+=(qttools5-dev libqt5scripttools5)
    INSTALLLIST+=(maslo-meteo-prognoz)
    SETUP_LIST+=(maslo_dlib.deb)    
    ;;
  [5] )
    echo "СПО Метеопродукция"
    SYSTEM_PKG_LIST+=(python3-lxml python3-pyqt5 python3-numpy libboost-python1.55.0)
    INSTALLLIST+=(maslo-meteo-product)
    SETUP_LIST+=(maslo_six.deb)
    ;;
  [6] )
    echo "СПО Погода"
    SYSTEM_PKG_LIST+=(postgresql-9.4 postgresql-client-9.4 postgresql-contrib-9.4 python3 libboost-python1.55.0 python3-numpy python3-protobuf apache2 libapache2-mod-wsgi libapache2-mod-auth-pam libapache2-mod-python)
    INSTALLLIST+=(maslo-meteo-www maslo-meteo-pogoda)
    SETUP_LIST+=(maslo_django.deb maslo_six.deb maslo_psycopg2.deb)
    ;;
  [7] )
    echo "СПО Взаимодействие"
    SYSTEM_PKG_LIST+=(postgresql-9.4 postgresql-client-9.4 postgresql-contrib-9.4 python3 libboost-python1.55.0 python3-numpy python3-protobuf apache2 libapache2-mod-wsgi libapache2-mod-auth-pam libapache2-mod-python)
    INSTALLLIST+=(maslo-meteo-www maslo-meteo-inter)
    SETUP_LIST+=(maslo_django.deb maslo_six.deb maslo_psycopg2.deb)
    ;;
  [8] )
    echo "DONE"
    ;;
  * )
    echo "СПО не выбрано, завершение работы программы"
    exit -1;
    ;;
esac
echo "Версия устанавливаемых пакетов:" $MAJOR_VERSION.$MINIOR_VERSION.$REVISION

apt-get install --yes ${SYSTEM_PKG_LIST[*]} || exit 1

for PACKAGE in ${SETUP_LIST[@]}
do
  dpkg -i $SETUP_ROOT/$PACKAGE || exit 1
done

for PACKAGE in ${INSTALLLIST[@]}
do
  PACKAGE_FILE_NAME=${PACKAGE}_${MAJOR_VERSION}.${MINIOR_VERSION}.${REVISION}.deb  
  dpkg -i $MASLO_ROOT/$PACKAGE_FILE_NAME || exit 1
done

