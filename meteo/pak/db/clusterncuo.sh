#!/bin/bash
# Создание кластера Postgres

DIR=`dirname $0`
DIR=`realpath $DIR`

# Параметры скрипта
DB_HOST=localhost
DB_PORT=5435
DAEMON_USER=postgres
USER=synop
USER_PASSWORD='11123'

DB_LIST=(db_inter $DAEMON_USER)
PSQL_GROUP=postgres
PSQL="psql --host=$DB_HOST --port=$DB_PORT"
PSQL_VERSION=9.6
CLUSTER_NAME=ncuo


# ВРЕМЕННО, потом удалить!
pg_dropcluster 9.6 $CLUSTER_NAME --stop


# Константы
EXIT_SUCCESS=0
EXIT_FAILTURE=1

ct_red="\e[31m"        #red
ct_yel="\033[33;40m"   #yellow
ct_green="\033[32;40m" #green
ct_blue="\033[34;40m"  #blue

c_def="\033[0m"         # default
c_status=$ct_blue       # status
c_ok=$ct_green          # ok
c_err=$ct_red           # error
c_wrn=$ct_yel           # warning

function f_error_log()
{
  echo -e "${c_err}[ ОШИБКА ]${c_def} ${@}"
}

function f_warning_log()
{
  echo -e "${c_wrn}[ ПРЕДУПРЕЖДЕНИЕ ]${c_def} ${@}"
}

function f_success_log()
{
  echo -e "${c_ok}[ УСПЕШНО ]${c_def} ${@}"
}

function f_status_log()
{
  echo -e "${c_status}[ ИНФОРМАЦИЯ ]${c_def} ${@}"
}

if ! getent group | grep $PSQL_GROUP
then
  f_error_log "Группа $PSQL_GROUP не существует"
  exit $EXIT_FAILTURE
fi

# Сам скрипт
if ! groups $DAEMON_USER | grep $PSQL_GROUP &> /dev/null
then
  f_status_log "Добавление пользователя $DAEMON_USER в группу $PSQL_GROUP"
  addgroup  $DAEMON_USER $PSQL_GROUP &> /dev/null
  if [[ $? == 0 ]]
  then
    f_success_log "Пользователь $DAEMON_USER успешно добавлен в группу $PSQL_GROUP"
  else
    f_error_log "Не удалось добавить пользователя $DAEMON_USER в группу $PSQL_GROUP"
    exit 1
  fi
else
  f_status_log "Пользователь $DAEMON_USER уже является членом группы $PSQL_GROUP"
fi


f_status_log "Установка требуемых разрешений файловой системы для авторизации пользователей в СУБД Postgresql"
#
if uname -a | grep astra
then
  setfacl -d -m g:$PSQL_GROUP:r /etc/parsec/macdb
  setfacl -R -m g:$PSQL_GROUP:r /etc/parsec/macdb
  setfacl -m g:$PSQL_GROUP:rx /etc/parsec/macdb
fi

# Создаем кластер
if [[ `pg_lsclusters  | awk '{print $2}' | grep $CLUSTER_NAME | wc -l` != 0 ]]
then
  if pg_dropcluster $PSQL_VERSION $CLUSTER_NAME --stop
  then
    f_status_log "Старый кластер СПО успешно очищен"
  fi
fi

if pg_createcluster $PSQL_VERSION $CLUSTER_NAME -p $DB_PORT --user=$DAEMON_USER
then
  f_success_log "Кластер СПО-2 успешно создан"
else
  f_error_log "Не удалось создать класер СПО-2"
  exit $EXIT_FAILTURE
fi

if [ ! -d "/etc/postgresql/$PSQL_VERSION/$CLUSTER_NAME" ]
then
  f_error_log "Каталог кластера Postgres /etc/postgresql/$PSQL_VERSION/$CLUSTER_NAME не обнаружен"
  exit 1
fi

cd /etc/postgresql/$PSQL_VERSION/$CLUSTER_NAME

mv pg_hba.conf pg_hba.conf.bak &&
echo "local   all             vgmdaemon                               trust" >> pg_hba.conf &&
echo "local   all             all                                     trust" >> pg_hba.conf &&
echo "host    all             all             0.0.0.0/0               trust" >> pg_hba.conf &&
echo "host    all             all             ::1/128                 trust" >> pg_hba.conf


if [[ "$?" == 0 ]]
then
  f_success_log "Настройки кластера изменены"
else
  f_error_log "Не удается изменить настройки кластера"
  exit $EXIT_FAILTURE
fi

# рестартим postgresql
f_status_log "Запуск СУБД Postgresql"

if /etc/init.d/postgresql restart $PSQL_VERSION $CLUSTER_NAME &> /dev/null
then
  f_success_log "СУБД Postgresql успешно запущена"
else
  f_error_log "Не удается запустить СУБД Postgresql"
  exit $EXIT_FAILTURE
fi

createuser -p $DB_PORT -l -d -U postgres $USER
# psql -p $DB_PORT -U postgres -c "alter user $USER with encrypted password '$USER_PASSWORD';"
# psql -p $DB_PORT -U postgres -c "create role $USER with login password '$USER_PASSWORD';"

# if ! groups $USER | grep $PSQL_GROUP &> /dev/null
# then
#   f_status_log "Добавление пользователя $USER в группу $PSQL_GROUP"
#   addgroup  $USER $PSQL_GROUP &> /dev/null
#   if [[ $? == 0 ]]
#   then
#     f_success_log "Пользователь $USER успешно добавлен в группу $PSQL_GROUP"
#   else
#     f_error_log "Не удалось добавить пользователя $USER в группу $PSQL_GROUP"
#     exit 1
#   fi
# else
#   f_status_log "Пользователь $USER уже является членом группы $PSQL_GROUP"
# fi

# psql -p 5435 -U postgres -c "create role gms with login password '11123';"

createdb -O $USER mrl -U postgres -p $DB_PORT
createdb -O $USER gmi -U postgres -p $DB_PORT
createdb -O $USER geo -U postgres -p $DB_PORT
createdb -O $USER arc -U postgres -p $DB_PORT

cd $DIR/dump/ncuo

pg_restore -h localhost -F t  -p $DB_PORT -U $USER -d gmi -C -v ./gmi.tar
pg_restore -h localhost -F t  -p $DB_PORT -U $USER -d geo -C -v ./geo.tar
pg_restore -h localhost -F t  -p $DB_PORT -U $USER -d arc -C -v ./arc.tar
pg_restore -h localhost -F t  -p $DB_PORT -U $USER -d mrl -C -v ./mrl.tar

psql -p $DB_PORT -U postgres -c "grant all privileges on database gmi to $USER;"
psql -p $DB_PORT -U postgres -c "grant all privileges on database geo to $USER;"
psql -p $DB_PORT -U postgres -c "grant all privileges on database arc to $USER;"
psql -p $DB_PORT -U postgres -c "grant all privileges on database mrl to $USER;"

# Устанавливаем пароль для Vgmdaemon
# if $PSQL -U $DAEMON_USER -tAc "ALTER USER $DAEMON_USER WITH PASSWORD '1';"
# then
#   f_success_log "Пароль пользователя $DAEMON_USER сброшен к значению '1'. Рекомендуется заменить данный пароль"
# else
#   f_warning_log "Не удалось изменить пароль пользователя $DAEMON_USER."
# fi

# Восстанавливаем настройки Postgres
# cd /etc/postgresql/$PSQL_VERSION/$CLUSTER_NAME
# if mv pg_hba.conf.bak pg_hba.conf && /etc/init.d/postgresql restart $PSQL_VERSION $CLUSTER_NAME &> /dev/null
# then
#   f_success_log "Настройки БД успешно сброшены"
# else
#   f_warning_log "Не удалось сбросить настройки БД. Проверьте правильность текущих настроек"
# fi
