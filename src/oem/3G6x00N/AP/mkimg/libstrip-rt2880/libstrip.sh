
RAMFSDIR=$1

OBJ_FILES=`find $RAMFSDIR \( -type d -o -type f -o -type b -o -type c -o -type l \) -print | file -f - | grep ELF |  grep "dynamically linked" | cut -d':' -f1`

echo $OBJ_FILES > temp

./libstrip $OBJ_FILES
