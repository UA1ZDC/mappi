#!/bin/sh
#�������� ������� ����� ~/.rpmmacros. 
#���� ��� ���, �� ����� ������. 
#���� �� ���� � �������� ������ �� ���������� �� ������������� �����������, �� ������ �� �������ģ�
#���� �� ���� � �������� �� ���������, ����� ������ �������������� � ���Σ��� ��� ������ $ERR_PARAM.


ERR_PARAM=66

file=$HOME/.rpmmacros

macros=(%_topdir %_unpackaged_files_terminate_build %_build_prefix %buildroot)
#mac_val=($HOME/build_dir/ 0 /usr/local %{_topdir})
mac_val=("%(echo \$BUILD_DIR)" 0 $PRJ_DIR %{_topdir})
count=${#macros[*]}

rm -f "$file"

if [ ! -e $file ]; then
#������� ����, ���� ��� ���.
  echo "Create file $file"
  for a in `seq $count`;
    do
    let "idx=a-1"
    echo ${macros[$idx]}  ${mac_val[$idx]} >> $file
  done

#else 
#���� ���� ����, ��������� ���������� ����������
# echo "Check $file"
#  for a in `seq $count`;
#     do
#     let "idx=a-1"
#  #    val=`awk -v mac="${macros[$idx]}" '$1==mac {print $0}' $file | cut -f 2- -d ' '`
#     val=`awk -v mac="${macros[$idx]}" '$1==mac  {if (NF==2) {print $2} else {print $2" " $3}}' $file`
# #    echo "val=_ $val _"
# #    echo ${macros[$idx]}  ${mac_val[$idx]}
#     if [ "$val" != "${mac_val[$idx]}" ]; then
#       echo "Error: unsuitable $file. Check  ${macros[$idx]} param."
#       exit $ERR_PARAM
#     fi
#   done
#  echo "Checking $file done."

fi

exit 0
