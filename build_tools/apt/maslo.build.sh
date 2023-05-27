#!/bin/bash
unset DISPLAY

if [ -d ~/build_dir ] 
then
  rm -r ~/build_dir
fi
rm '/share/ftp/maslo/pool/maslo/*.deb'

cd ${SRC}/meteo/maslo
git pull
#./build.sh clean -a || exit 1
./build.sh || exit 1 
./build.sh mkdeb -nb --prefix=/opt/maslo --type=develop || exit 1
mv ~/build_dir/package/*.deb /share/ftp/maslo/pool/maslo/

echo Обновление репозитория открытого сегмента
ssh vgmdaemon@172.16.0.1 'if [ -d /home/vgmdaemon/maslo ]; then rm -r /home/vgmdaemon/maslo; fi' || exit 1
ssh vgmdaemon@172.16.0.1 'if [ -d /share/ftp/maslo/pool/maslo ]; then rm -r /share/ftp/maslo/pool/maslo; fi' || exit 1
scp -r /share/ftp/maslo/pool/maslo/ vgmdaemon@172.16.0.1:/home/vgmdaemon || exit 1
ssh vgmdaemon@172.16.0.1 'mv /home/vgmdaemon/maslo /share/ftp/maslo/pool/' || exit 1
ssh vgmdaemon@172.16.0.1 'bash /share/ftp/maslo/update.sh' || exit 1

echo обновление репозитория закрытого сегмента
ssh vgmdaemon@172.16.1.1 'if [ -d /home/vgmdaemon/maslo ]; then rm -r /home/vgmdaemon/maslo; fi' || exit 1
ssh vgmdaemon@172.16.1.1 'if [ -d /share/ftp/maslo/pool/maslo ]; then rm -r /share/ftp/maslo/pool/maslo; fi' || exit 1
scp -r /share/ftp/maslo/pool/maslo/ vgmdaemon@172.16.1.1:/home/vgmdaemon || exit 1
ssh vgmdaemon@172.16.1.1 'mv /home/vgmdaemon/maslo /share/ftp/maslo/pool/' || exit 1
ssh vgmdaemon@172.16.1.1 'bash /share/ftp/maslo/update.sh' || exit 1

echo обновление банк открытый сегмент
ssh vgmdaemon@172.16.0.1 'if [ -f /etc/init.d/maslo]; then sudo /etc/init.d/maslo stop; fi'
ssh vgmdaemon@172.16.0.1 'sudo apt-get purge --yes maslo*' || exit 1
ssh vgmdaemon@172.16.0.1 'if [ -d /opt/maslo ]; then sudo rm -r /opt/maslo; fi' || exit 1
ssh vgmdaemon@172.16.0.1 'sudo apt-get update'
ssh vgmdaemon@172.16.0.1 'ls /opt'
ssh vgmdaemon@172.16.0.1 'sudo apt-get install --yes  maslo-bank-settings maslo-meteo-bank maslo-meteo-inter maslo-meteo-pogoda maslo-meteo-product maslo-meteo-prognoz maslo-meteo-sbor' || exit 1
ssh vgmdaemon@172.16.0.1 'sudo /etc/init.d/maslo start' || exit 1

echo обвновление радуга 1 открытый сегмент
ssh vgmdaemon@172.16.1.3 'if [ -f /etc/init.d/maslo]; then sudo /etc/init.d/maslo stop; fi'
ssh vgmdaemon@172.16.1.3 'sudo apt-get purge --yes maslo*' || exit 1
ssh vgmdaemon@172.16.1.3 'if [ -d /opt/maslo ]; then sudo rm -r /opt/maslo; fi' || exit 1
ssh vgmdaemon@172.16.1.3 'sudo apt-get update'
ssh vgmdaemon@172.16.1.3 'ls /opt'
ssh vgmdaemon@172.16.1.3 'sudo apt-get install --yes  maslo-meteo-bank maslo-meteo-bankdb maslo-meteo-prognoz maslo-meteo-product maslo-meteo-sbor maslo-raduga1-settings' || exit 1
ssh vgmdaemon@172.16.1.3 'sudo /etc/init.d/maslo start' || exit 1

echo обвновление радуга 2 открытый сегмент
ssh vgmdaemon@172.16.1.4 'if [ -f /etc/init.d/maslo]; then sudo /etc/init.d/maslo stop; fi'
ssh vgmdaemon@172.16.1.4 'sudo apt-get purge --yes maslo*' || exit 1
ssh vgmdaemon@172.16.1.4 'if [ -d /opt/maslo ]; then sudo rm -r /opt/maslo; fi' || exit 1
ssh vgmdaemon@172.16.1.4 'sudo apt-get update'
ssh vgmdaemon@172.16.1.4 'ls /opt'
ssh vgmdaemon@172.16.1.4 'sudo apt-get install --yes  maslo-meteo-bank maslo-meteo-bankdb maslo-meteo-prognoz maslo-meteo-product maslo-meteo-sbor maslo-raduga2-settings' || exit 1
ssh vgmdaemon@172.16.1.4 'sudo /etc/init.d/maslo start' || exit 1

echo обновление банк закрытый сегмент
ssh vgmdaemon@172.16.1.1 'if [ -f /etc/init.d/maslo]; then sudo /etc/init.d/maslo stop; fi'
ssh vgmdaemon@172.16.1.1 'sudo apt-get purge --yes maslo*' || exit 1
ssh vgmdaemon@172.16.1.1 'if [ -d /opt/maslo ]; then sudo rm -r /opt/maslo; fi' || exit 1
ssh vgmdaemon@172.16.1.1 'sudo apt-get update'
ssh vgmdaemon@172.16.1.1 'ls /opt'
ssh vgmdaemon@172.16.1.1 'sudo apt-get install --yes  maslo-bankzs-settings maslo-meteo-bank maslo-meteo-inter maslo-meteo-pogoda maslo-meteo-product maslo-meteo-prognoz maslo-meteo-sbor' || exit 1
ssh vgmdaemon@172.16.1.1 'sudo /etc/init.d/maslo start' || exit 1

echo обвновление радуга 1 закрытый сегмент
ssh vgmdaemon@172.16.0.3 'if [ -f /etc/init.d/maslo]; then sudo /etc/init.d/maslo stop; fi'
ssh vgmdaemon@172.16.0.3 'sudo apt-get purge --yes maslo*' || exit 1
ssh vgmdaemon@172.16.0.3 'if [ -d /opt/maslo ]; then sudo rm -r /opt/maslo; fi' || exit 1
ssh vgmdaemon@172.16.0.3 'sudo apt-get update'
ssh vgmdaemon@172.16.0.3 'ls /opt'
ssh vgmdaemon@172.16.0.3 'sudo apt-get install --yes  maslo-meteo-bank maslo-meteo-bankdb maslo-meteo-prognoz maslo-meteo-product maslo-meteo-sbor maslo-raduga1zs-settings' || exit 1
ssh vgmdaemon@172.16.0.3 'sudo /etc/init.d/maslo start' || exit 1

echo обвновление радуга 2 закрытый сегмент
ssh vgmdaemon@172.16.0.4 'if [ -f /etc/init.d/maslo]; then sudo /etc/init.d/maslo stop; fi'
ssh vgmdaemon@172.16.0.4 'sudo apt-get purge --yes maslo*' || exit 1
ssh vgmdaemon@172.16.0.4 'if [ -d /opt/maslo ]; then sudo rm -r /opt/maslo; fi' || exit 1
ssh vgmdaemon@172.16.0.4 'sudo apt-get update'
ssh vgmdaemon@172.16.0.4 'ls /opt'
ssh vgmdaemon@172.16.0.4 'sudo apt-get install --yes  maslo-meteo-bank maslo-meteo-bankdb maslo-meteo-prognoz maslo-meteo-product maslo-meteo-sbor maslo-raduga2zs-settings' || exit 1
ssh vgmdaemon@172.16.0.4 'sudo /etc/init.d/maslo start' || exit 1

echo обвновление радугаП 2 закрытый сегмент
ssh vgmdaemon@172.16.2.2 'if [ -f /etc/init.d/maslo]; then sudo /etc/init.d/maslo stop; fi'
ssh vgmdaemon@172.16.2.2 'sudo apt-get purge --yes maslo*' || exit 1
ssh vgmdaemon@172.16.2.2 'if [ -d /opt/maslo ]; then sudo rm -r /opt/maslo; fi' || exit 1
ssh vgmdaemon@172.16.2.2 'sudo apt-get update'
ssh vgmdaemon@172.16.2.2 'ls /opt'
ssh vgmdaemon@172.16.2.2 'sudo apt-get install --yes  maslo-meteo-bank maslo-meteo-bankdb maslo-meteo-prognoz maslo-meteo-product maslo-meteo-sbor maslo-radugap2-settings' || exit 1
ssh vgmdaemon@172.16.2.2 'sudo /etc/init.d/maslo start' || exit 1

echo обвновление радугаП 1 закрытый сегмент
ssh vgmdaemon@172.16.2.1 'if [ -f /etc/init.d/maslo]; then sudo /etc/init.d/maslo stop; fi'
ssh vgmdaemon@172.16.2.1 'sudo apt-get purge --yes maslo*' || exit 1
ssh vgmdaemon@172.16.2.1 'if [ -d /opt/maslo ]; then sudo rm -r /opt/maslo; fi' || exit 1
ssh vgmdaemon@172.16.2.1 'sudo apt-get update'
ssh vgmdaemon@172.16.2.1 'ls /opt'
ssh vgmdaemon@172.16.2.1 'sudo apt-get install --yes  maslo-meteo-bank maslo-meteo-bankdb maslo-meteo-prognoz maslo-meteo-product maslo-meteo-sbor maslo-radugap1-settings' || exit 1
ssh vgmdaemon@172.16.2.1 'sudo /etc/init.d/maslo start' || exit 1