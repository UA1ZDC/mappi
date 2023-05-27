#!/bin/sh
#������ rpm ������

if [ $# -ne 1 ]; then
  echo "������������� ���������: `basename $0` <��� ������>"
  exit 65
fi

#��� spec-����� ������
pack=$1

mkdir -p $BUILD_DIR/RPMS
str=`rpmbuild -bb $pack 2>&1`
err=$?
if [ $err -ne 0 ]; then
  echo "$str"
fi

exit $err
