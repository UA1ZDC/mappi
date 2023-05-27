#!/bin/bash

CD=`pwd`

$CD/build.sh fullclean-all
if [ $? -ne 0 ]; then
  echo "Error: clean"
  exit 1
fi

find $CD -name '.o'   -exec rm -rf {} \; 2>/dev/null
find $CD -name '.moc' -exec rm -rf {} \; 2>/dev/null
find $CD -name '.ui'  -exec rm -rf {} \; 2>/dev/null
find $CD -name '.rcc' -exec rm -rf {} \; 2>/dev/null

$CD/build.sh build
if [ $? -ne 0 ]; then
  echo "Error: build"
  exit 4
fi

$CD/build.sh autotests-all
if [ $? -ne 0 ]; then
  echo "Error: build"
  exit 4
fi

$CD/build.sh run-autotests-all
if [ $? -ne 0 ]; then
  echo "Error: tests"
  exit 2
fi

exit 0

