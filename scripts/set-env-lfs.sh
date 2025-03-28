export LFS="$HOME/Desktop/LFS/mnt/lfs"
read -p "Enter device number to mount: " part_id
mount -v -t ext4 "/dev/sda$part_id"

chown root:root $LFS
chmod 755 $LFS

if test -d $LFS/home ; then

	echo "$LFS/home dir: OK"


else

	echo "$LFS/home dir: NOT FOUND"
	echo "Making HOME dir in $LFS..."
	mkdir -v $LFS/home

fi

if test -d $LFS/sources ; then

	echo "$LFS/sources dir: OK"

else

	echo "$LFS/home dir: NOT FOUND"
	echo "Making SOURCES dir in $LFS..."

	mkdir -v $LFS/sources
	chmod -v a+wt $LFS/sources
	chown root:root $LFS/sources/*

fi

