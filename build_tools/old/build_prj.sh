#!/bin/sh
#Создание набора пакетов указанного проекта для последующей установки и укладываение их в $BUILD_DIR/tar/<имя проекта>

ERR_ARGS=65

CLI_OPT=client
SRV_OPT=server

if [ $# -lt 1 ]; then
  echo "использование программы: `basename $0` <имя проекта> [файл со списком пакетов] [архтектура]"
  exit 65
fi

#имя проекта
name=$1
#файл со списком пакетов
#prj=${2:-$SRC/pack_lists/${name}_list}
prj=${2:-${BUILD_DIR}/pack_lists/${name}_list}
echo $prj
#архитектура
arch=${3:-`rpm --eval "%{_arch}"`}


tar_dir=$BUILD_DIR/tar/

if [ ! -f $prj ];then
  exit $ERR_ARGS
fi

pack_opt=`echo ${name}_PACK_OPT | tr '[a-z]' '[A-Z]'`
pack_opt=${!pack_opt}

case $pack_opt in
  $CLI_OPT)
    pack_name=${name}_client
    ;;
  $SRV_OPT)
    pack_name=${name}_server
    ;;
  *)
    pack_name=${name}
    ;;
esac

mkdir -p $tar_dir/tmp/$pack_name
rm   -rf $tar_dir/tmp/$pack_name/*

#cp $prj $tar_dir/tmp/$pack_name/
sed "s/@ARCH@/$arch/g" $prj > $tar_dir/tmp/$pack_name/${name}_list

#rm -rf $tar_dir/tmp/$pack_name/${name}_list

while read line ; do
  case $line in
  '#'* | '')
    continue;;
  esac
  case $pack_opt in
    $CLI_OPT)
      if [ "$line" != "${line% $SRV_OPT}" ]; then
        continue
      fi
      ;;
    $SRV_OPT)
      if [ "$line" != "${line% $CLI_OPT}" ]; then
        continue
      fi
      ;;
    *)
      ;;
  esac

  line=${line% $SRV_OPT}
  line=${line% $CLI_OPT}
  echo $line >> $tar_dir/tmp/$pack_name/${name}_list.tmp
  rpmname=${line% *}
  cp $BUILD_DIR/RPMS/$arch/$rpmname $tar_dir/tmp/$pack_name
done < $tar_dir/tmp/$pack_name/${name}_list
mv $tar_dir/tmp/$pack_name/${name}_list.tmp $tar_dir/tmp/$pack_name/${name}_list


sed "s/@PACK@/$name/g"  $SRC/build_tools/install_prj.sh > $tar_dir/tmp/$pack_name/install-sh
chmod 755 $tar_dir/tmp/$pack_name/install-sh

pack=`awk '{ print $1 }' $tar_dir/tmp/$pack_name/${name}_list | tac`
pack=`echo $pack` #без этой строки sed не хочет работать. как-то tac не так возвращает список"
sed "s/@PACK@/$pack/g"  $SRC/build_tools/remove_prj.sh  > $tar_dir/tmp/$pack_name/uninstall-sh 
chmod 755 $tar_dir/tmp/$pack_name/uninstall-sh


cd $tar_dir/tmp
tar -cf $tar_dir/$pack_name.tar $pack_name
cd -

cp -r $tar_dir/tmp/$pack_name $tar_dir/

rm -rf mkdir $tar_dir/tmp

exit 0
