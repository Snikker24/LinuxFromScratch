
#!/bin/bash

#Variables

RED='\033[0;31m'
LRED='\033[1;31m'
ORANGE='\033[0;33m'
YELLOW='\033[1;93m'
BLUE='\033[0;34m'
LBLUE='\033[0;94m'
GREEN='\033[0;32m'
DEFAULT='\033[0m'
BOLD='\033[1m'



#Function declaration


mount_root_part(){

	local raw=$( lsblk -o NAME,SIZE,FSTYPE,MOUNTPOINT)

	NEWLINE=$'\n'

	OIFS= $IFS
	IFS=$NEWLINE

	local devlist=($raw)

	IFS=' ';

	local devstr=''


	local k=0;
	local device
	for i in "${devlist[@]}"
	do
		#echo $i;

		read -a device <<< $i


		#device[0]=$( echo ${device[0]} | sed "s/\W*//" )
		device[0]=${device[0]//[^[:alnum:]]/}


		#echo ${device[0]}
		#echo ${device[1]}
		#echo ${device[2]}

		
		if [[ ${device[3]} != '[SWAP]' && ${device[2]} != "" ]]; then
		
			if (( k!=0 )); then

				#if ((k<=${#devlist[@]}/3));then
					devstr="${devstr}${GREEN}${k}${DEFAULT}	${device[0]}	${device[1]}	${device[2]}	${device[3]}${NEWLINE}"
				#else
					#devstr="${devstr}${k} ${device[0]} ${device[1]} ${device[2]}"
				#fi

			else
				devstr="${GREEN}${devstr}ID	${device[0]}	${device[1]}	${device[2]}	${device[3]}${DEFAULT}${NEWLINE}"
				
			fi

			k=$((k+1));
		fi

	done

	devstr="${devstr}${NEWLINE}${ORANGE}0 Cancel${DEFAULT} ${NEWLINE}"

	
	echo -e "${NEWLINE}${YELLOW}${BOLD}WARNING:${DEFAULT} Be careful! Choosing the wrong partition for LinuxFromScratch can break your system!"
	echo -e "${YELLOW}${BOLD}WARNING:${DEFAULT} Avoid using the host OS partitions (ROOT, BOOT, EFI, HOME, ...) for the guest root partition."$NEWLINE
	echo -e "${LBLUE}${BOLD}INFO:${DEFAULT} suitable partitions found: $((k-1))"$NEWLINE
	echo -e $devstr

	read -p "Enter partition ID to mount: " id

	while [[ !$id=~'^[0-9]+$' && $id<0  || $id>$((k-1)) ]]; do

		echo -e "${RED}${BOLD}ERROR:${DEFAULT} Not a valid partition ID! Try again..."
		read -p "Enter partition ID to mount: " id
	done


	if (($id==0)); then
		IFS=$OIFS
		exit
	else

		IFS=$NEWLINE

		devlist=($devstr)

		IFS="	"

		read -a device <<< ${devlist[$id]}

		echo -e "${LBLUE}${BOLD}INFO:${DEFAULT} Partition $id (/dev/${device[1]} | ${device[3]})) has been selected. Mounting..."

		sudo mount -v -t $device[2] "/dev/${device[1]}" $LFS

	fi

	IFS=$OIFS

}

setup_lfs_home(){

	export LFS_HOME="$1"

}

setup_lfs_env(){

	export LFS="$LFS_HOME/mnt/lfs"
	umask 022
	mkdir $LFS_HOME
	mkdir "$LFS_HOME/mnt"
	mkdir "$LFS_HOME/mnt/lfs"

}


setup_directory(){

	if test -d $LFS/$1; then
		echo "$LFS/$1 | ${GREEN}${BOLD}[OK]${DEFAULT}"
	else
		echo -e "$LFS/$1 | ${YELLOW}${BOLD}[NOT FOUND]${DEFAULT}"
		echo "${LBLUE}${BOLD}Creating directory...${DEFAULT}"
		mkdir -v $LFS/home
	fi

}

mount_root_part