#!/bin/bash

#
# Использование: ./dumpall.sh -h HOST -p PORT [-d DB_NAME]
#

DUMP_DIR=$SRC/mappi/db

DB_HOST="127.0.0.1"
DB_PORT="5432"
DB_NAME=()


for_STATE="none"
for i in "$@"
do
case $i in
    -h)
        for_STATE="host"
        shift
    ;;
    -p)
        for_STATE="port"
        shift
    ;;
    -d)
        for_STATE="db_name"
        shift
    ;;
    *)
        if [[ "db_name" == "${for_STATE}" ]]; then
          DB_NAME+=($i)
        elif [[ "host" == "${for_STATE}" ]]; then
          DB_HOST=$i
        elif [[ "port" == "${for_STATE}" ]]; then
          DB_PORT=$i
        fi
        shift
    ;;
esac
done

if [[ 0 == ${#DB_NAME[@]} ]]; then
  DB_NAME+=("mappidb")
fi

declare -A DUMP_SCHE_PARAMS
DUMP_SCHE_PARAMS["mappidb"]="-T *_20*"

declare -A DUMP_DATA_PARAMS
DUMP_DATA_PARAMS["mappidb"]="-T *_20*"

for db in "${DB_NAME[@]}"
do
    DUMP_FILE="$DUMP_DIR/scheme.$db.orig.sql"

    pg_dump -h $DB_HOST -p $DB_PORT -d $db -U postgres -s -f $DUMP_FILE --format=p ${DUMP_SCHE_PARAMS[$db]}
    if [[ 0 != $? ]]; then res="[fail]"; else res="[ ok ]"; fi
    echo "$res [$DB_HOST:$DB_PORT] $db"
done

for db in "${DB_NAME[@]}"
do
    DUMP_FILE="$DUMP_DIR/data.$db.orig.sql"

    pg_dump -h $DB_HOST -p $DB_PORT -d $db -U postgres -a --column-inserts -f $DUMP_FILE --format=p ${DUMP_DATA_PARAMS[$db]}
    if [[ 0 != $? ]]; then res="[fail]"; else res="[ ok ]"; fi
    echo "$res [$DB_HOST:$DB_PORT] $db"
done

. $DUMP_DIR/replace_in_dump.sh
