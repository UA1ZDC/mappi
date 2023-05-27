#!/bin/bash

host="127.0.0.1"
port="5432"
db="all"

isDebian=`uname -a | grep Debian | wc -l`

while getopts ":h:p:d:A:" option
do
  case $option in
    h ) host=$OPTARG;;
    p ) port=$OPTARG;;
    d ) db=$OPTARG;;
  esac
done
shift $(($OPTIND - 1))


function run_psql
{
  host_=$1
  port_=$2
  db_=$3
  file_=$4
  
  echo "----- $db_ $file_ ---------------" >> /tmp/deploydb.log

  echo -n "[....] $db_ $file_"
  psql -v ON_ERROR_STOP=1 -q -x --single-transaction -U postgres -d $db_ -h $host_ -p $port_ -f $file_ >> /tmp/deploydb.log 2>&1
  if [[ 0 = $? ]]; then echo -e "\e[0K\r[ ok ]"; else echo -e "\e[0K\r[fail]";  exit 1; fi
}

# ------ MAIN --------

rm -f /tmp/deploydb.log

echo "DEPLOY DB [$host:$port]:"
echo "log in /tmp/deploydb.log"


#if [[ 0 -eq $isDebian ]]; then
 # run_psql $host $port mappidb   audit_role.sql
#fi


run_psql $host $port mappidb     scheme.mappidb.orig.sql
run_psql $host $port mappidb     data.mappidb.orig.sql
