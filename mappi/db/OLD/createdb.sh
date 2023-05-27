#!/bin/bash

DB_LIST=(mappidb)

echo "CREATE DB:"
for db in ${DB_LIST[@]}
do
  createdb -U postgres $db
  if [[ 0 = $? ]]; then echo "[ ok ] '$db'"; else echo "[fail] '$db'"; fi
done
