#!/bin/sh

#�������� ����������� ���� ��� ������ rpm-������

mkdir -p $BUILD_DIR/$PRJ_DIR
cp -r $BUILD_DIR/bin  $BUILD_DIR/$PRJ_DIR
cp -r $BUILD_DIR/etc  $BUILD_DIR/$PRJ_DIR
cp -r $BUILD_DIR/lib  $BUILD_DIR/$PRJ_DIR
cp -r $BUILD_DIR/sbin $BUILD_DIR/$PRJ_DIR
cp -r $BUILD_DIR/share $BUILD_DIR/$PRJ_DIR
cp -r $BUILD_DIR/var  $BUILD_DIR/$PRJ_DIR

exit 0
