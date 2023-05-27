#!/bin/bash

if [ $# -lt 1 ]; then
    echo "Неверное количество параметров"
    echo "$0 <filename>"
   exit -1
fi

host="127.0.0.1"
port="27017"

while getopts ":h:p:" option
do
  case $option in
    h ) host=$OPTARG;;
    p ) port=$OPTARG;;
  esac
done
shift $(($OPTIND - 1))

id="ObjectId('"$1"')"

echo mongofiles --host $host --port $port --db telegramsdb get_id $id
mongofiles --host $host --port $port --db telegramsdb get_id $id

exit 0
