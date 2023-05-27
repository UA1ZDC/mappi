#!/bin/bash

DB_LIST=(mappidb)

echo "DROP DB:"
for db in ${DB_LIST[@]}
do 
  dropdb -U postgres --if-exists $db 
  if [[ 0 = $? ]]; then echo -n "[ ok ] '$db'"; else echo -n "[fail] '$db'"; fi
  echo ""
done
