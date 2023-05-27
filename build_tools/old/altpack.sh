#!/bin/sh
# ���������� ���� �������� � ������ rpm

ERR_ARG=65
ERR_SRC=66

if [ -z $SRC ]; then
  echo "�� ���������� ���������� ��������� SRC"
  exit $ERR_SRC
fi

old_dir=`pwd`

cd $SRC
. env-bash system
if [ -z "$1" ]; then 
  echo "�� ������ ������, ��� �������� ���������� ������� rpm-������"
  exit $ERR_ARG
fi

#make altpack
#run script for build rpm
export BUILD_DIR=~/rpm_build
BUILD_RPM=${SRC}/build_tools/build_rpm.sh
BUILD_PATH=${SRC}/build_tools/rpmdirs.sh

rm -rf $BUILD_DIR/usr/local
${BUILD_PATH}
for spec_file in ${BUILD_DIR}/SPECS/*.spec;
do
  ${BUILD_RPM} $spec_file;
done

rm -rf ${BUILD_DIR}/usr/local;

cd $old_dir

exit 0
