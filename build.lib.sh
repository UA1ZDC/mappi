args=();
count=0;

while [ -n "$1" ];do
   case "$1" in
      --prefix )
         export PRJ_DIR=$2
         shift;
      ;;
      *  )
         args[$count]=$1;
         count=$((count+1));
   esac;
   shift;
done;

for n in ${args[@]}
do
  export TARGET=$n
  cmake -P build.cmake
  if [ $? -ne 0 ]; then
    return 1
  fi
done


if [[ ${#args[@]}  == 0 ]]
then
  export TARGET='build'
  cmake -P build.cmake
  if [ $? -ne 0 ]; then
    return 1
  fi
fi

