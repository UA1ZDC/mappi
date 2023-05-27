#!/bin/sh

#��������� �� ���� ����� � ������� �������� � ����������� �������� ��� �������� � �������

INSTALL="install-sh"
#UNINSTALL="uninstall-sh"

function install_prj()
{
  local curdir=`pwd`
  cd $1
  local pr_name=${1##*/}
  pr_name=${pr_name%.tar}
  if [ "" != "`ls | grep -w $INSTALL`" ]; then
    echo "��������� $pr_name"
    ./$INSTALL
    echo "***"
  fi
  cd $curdir  
}

for dir in * ; do 
  #���� �����, ��������� � ��� $INSTALL
  if [ -d "$dir" ]; then 
    install_prj $dir
  else
    #���� tar-�����, ����������� � ��������� $INSTALL
    type=`file $dir | grep "POSIX tar archive"`
    if [ "$type" != "" ]; then
      curdir=`pwd`
      cd /tmp
      tar -xf $curdir/$dir
      install_prj ${dir%.tar}
      rm -rf /tmp/$dir
      cd $curdir
    fi
  fi
done

exit 0
