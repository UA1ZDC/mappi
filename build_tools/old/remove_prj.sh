#!/bin/sh
#деинсталирует из системы все пакеты $pack

ERR_ARGS=65

pack=(@PACK@)

count=${#pack[*]}

for i in "${pack[@]}"
do
  rpm -e --nodeps --allmatches ${i%-*-*rpm}
done

exit 0
