
#!/bin/bash


clear
#Variables

RED='\033[0;31m'
ORANGE='\033[0;33m'
YELLOW='\033[1;93m'
BLUE='\033[0;34m'
LBLUE='\033[0;94m'
GREEN='\033[0;32m'
BLACK='\033[0;30m'

LREDBG='\033[30;41m'
GREENBG='\033[30;42m'

DEFAULT='\033[0m'
BOLD='\033[1m'

BLINK='\e[5m'

NEWLINE=$'\n'

#Title
display_title(){
	echo -e "${LBLUE}${BOLD}"
	echo '╔══════════════════════════════════════════════════════════════════════════════════╗'
	echo '║   __ _                    ___                    __                _       _     ║'
	echo '║  / /(_)_ __  _   ___  __ / __\ __ ___  _ __ ___ / _\ ___ _ __ __ _| |_ ___| |__  ║'
	echo "║ / / | | '_ \| | | \ \/ // _\| '__/ _ \| '_ \` _ \\\\ \ / __| '__/ _\` | __/ __| '_ \ ║"
	echo '║/ /__| | | | | |_| |>  </ /  | | | (_) | | | | | |\ \ (__| | | (_| | || (__| | | |║'
	echo '║\____/_|_| |_|\__,_/_/\_\/   |_|  \___/|_| |_| |_\__/\___|_|  \__,_|\__\___|_| |_|║'
	echo '╚══════════════════════════════════════════════════════════════════════════════════╝'
	echo -e "${GREEN}${BOLD}> LinuxFromScratch setup utility script${DEFAULT}${NEWLINE}"
}


#Function declaration


input_break(){
	echo -e "${NEWLINE}${GREENBG}${BLACk}${BOLD}Press any key to continue:${DEFAULT}"
	read -n 1 -s -r
}

mount_root_part(){

	local raw=$( lsblk -o NAME,SIZE,FSTYPE,MOUNTPOINT)


	local OIFS= $IFS
	IFS=$NEWLINE

	local pttype=($(lsblk -o SIZE,PARTTYPENAME))

	local z=0;
	for i in ${pttype[@]}
	do
		#pttype[$z]=${pttype[$z]:1}
		pttype[$z]=${i//[^[:ascii:][:space:]]/}
		pttype[$z]=${pttype[$z]:6}
		z=$((z+1))
	done

	local devlist=($raw)

	IFS=' ';

	local devstr=''


	local k=0;
	local device
	z=0
	for i in "${devlist[@]}"
	do
		#echo $i;

		read -a device <<< $i


		#device[0]=$( echo ${device[0]} | sed "s/\W*//" )
		device[0]=${device[0]//[^[:alnum:]]/}


		#echo ${device[0]}
		#echo ${device[1]}
		#echo ${device[2]}

		#echo $z${pttype[$z]}

		
		if [[ ${device[3]} != '[SWAP]' && ${device[2]} != "" ]]; then
		
			if (( k!=0 )); then

				#if ((k<=${#devlist[@]}/3));then
					devstr="${devstr}${LBLUE}${BOLD}${k}${DEFAULT}	${device[0]}	${device[1]}	${device[2]}	${pttype[$z]}		${device[3]}${NEWLINE}"
				#else
					#devstr="${devstr}${k} ${device[0]} ${device[1]} ${device[2]}"
				#fi

			else
				devstr="${GREEN}${BOLD}${devstr}ID	${device[0]}	${device[1]}	${device[2]}	PARTITION-TYPE		${device[3]}${DEFAULT}${NEWLINE}"
				
			fi

			k=$((k+1));
		fi
		z=$((z+1))

	done

	k=$((k-1));
	devstr="${devstr}${NEWLINE}${ORANGE}${BOLD}0 Cancel (CTRL+C)${DEFAULT} ${NEWLINE}"

	if [[ $k>0 ]]; then

		echo -e "${NEWLINE}${YELLOW}${BOLD}WARNING:${DEFAULT} Be careful! Choosing the wrong partition for LinuxFromScratch can break your system!"
		echo -e "${YELLOW}${BOLD}WARNING:${DEFAULT} Avoid using the host OS partitions (ROOT, BOOT, SWAP, EFI, HOME, ...) as the main root partition."$NEWLINE
		echo -e "${LBLUE}${BOLD}INFO:${DEFAULT} suitable partitions found: $k"$NEWLINE
		echo -e $devstr
		#echo -e "${YELLOW}${BOLD}[!]${DEFAULT} Enter partition ID to mount:"
		
		read -p "$(echo -e "${YELLOW}${BOLD}[!]${DEFAULT} Enter partition ID to mount:")" id

		while [[ !$id=~'^[0-9]+$' && $id<0  || $id>$k ]]; do

			echo -e "${RED}${BOLD}ERROR:${DEFAULT} Not a valid partition ID! Try again..."
			#echo -e "${YELLOW}${BOLD}[!]${DEFAULT} Enter partition ID to mount:"
			read -p "$(echo -e "${YELLOW}${BOLD}[!]${DEFAULT} Enter partition ID to mount:")" id

		done


		if (($id==0)); then
			IFS=$OIFS
			echo -e "${LBLUE}${BOLD}INFO:${DEFAULT} Operation cancelled. Exiting..."
			exit
		else

			IFS=$NEWLINE

			devlist=($devstr)

			IFS="	"

			read -a device <<< ${devlist[$id]}

			echo -e "${LBLUE}${BOLD}INFO:${DEFAULT} Partition $id (/dev/${device[1]} | ${device[2]} | ${device[3]})) has been selected. Mounting..."

			LFS_MOUNT="/dev/${device[1]}"
			LFS_FS=${device[3]}

			sudo mount -v -t $LFS_FS $LFS_MOUNT $LFS


		fi

	else

		echo -e "${RED}${BOLD}ERROR:${DEFAULT} No suitable partitions found!"
		echo -e "${RED}${BOLD}ERROR:${DEFAULT} Please create a new partition and run the script again."
		echo -e "${RED}${BOLD}ERROR:${DEFAULT} Exiting..."
		exit
	fi

	IFS=$OIFS

}

download_packages(){

	local OIFS=$IFS

	IFS=$NEWLINE


	target_dir=$2

	packages=$(<$1)
	packages=($packages)

	IFS=$OIFS

	local i=1
	len=${#packages[@]}
	for pack in ${packages[@]}
	do

		IFS="/"
		packname=($pack)
		IFS=$OIFS

		packname=${packname[$((${#packname[@]}-1))]}
		echo -en "${GREEN}${BOLD}Downloading package: ${ORANGE} $packname [$i/$len]${DEFAULT}"
		wget -q -P "$target_dir/" $pack
		echo -e " ${LBLUE}${BOLD}DONE${DEFAULT}"

		i=$((i+1))

	done


}

setup_lfs(){

	display_title
	input_break

	local config=$(<config.txt)
	
	local OIFS=$IFS
	IFS=$NEWLINE

	local vars
	vars=($config)

	IFS="="

	echo -e "${NEWLINE}${LBLUE}${BOLD}INFO:${DEFAULT} Generating variables from config.txt"
	
	for var in "${vars[@]}"
	do
	
		local varmap
		read -a varmap <<< $var
	
		case ${varmap[0]} in 
		
			"LFS_HOME")

				LFS_HOME=${varmap[1]}
				LFS="$(eval "echo \"$LFS_HOME\"")/mnt/lfs"
				echo -e "${GREEN}${BOLD}[${varmap[0]}]${DEFAULT} $(eval "echo \"$LFS_HOME\"")"
				echo -e "${GREEN}${BOLD}[LFS]${DEFAULT} $LFS"
				;;

			"LFS_MOUNT")

				LFS_MOUNT=${varmap[1]}
				echo -e "${GREEN}${BOLD}[${varmap[0]}]${DEFAULT} $(eval "echo \"$LFS_MOUNT\"")"
				;;

			"LFS_FS")

				LFS_FS=${varmap[1]}
				echo -e "${GREEN}${BOLD}[${varmap[0]}]${DEFAULT} $(eval "echo \"$LFS_FS\"")"
				;;

		esac

	done

	IFS=$OIFS

	echo -e "${NEWLINE}${YELLOW}${BOLD}WARNING:${DEFAULT} Check that the settings are corresponding to your desired configuration before continuing."

	input_break

	umask 022

	echo -e "${NEWLINE}${LBLUE}${BOLD}INFO:${DEFAULT} Checking LFS home structure..."

	setup_directory "$(eval "echo \"$LFS_HOME\"")"
	setup_directory "$(eval "echo \"$LFS_HOME\"")/mnt"
	setup_directory "${LFS}"

	if [[ -z $LFS_HOME ]]; then

		LFS_HOME="\$HOME/Desktop/LFS"
		LFS="$(eval "echo \"$LFS_MOUNT\"")/mnt/lfs"

		echo "LFS_HOME=${LFS_HOME}">>config.txt

	fi

	export LFS_HOME
	export LFS

	echo -e "${NEWLINE}${LBLUE}${BOLD}INFO:${DEFAULT} Mounting root partition."

	if [[ -z $LFS_MOUNT || -z $LFS_FS ]]; then


		echo -e "${NEWLINE}${LREDBG}${BOLD}Could not find root partition in config.txt${DEFAULT}"
		echo -e "${LREDBG}${BOLD}You must specify a root partition for LFS.${DEFAULT}"

		input_break

		mount_root_part

		echo "LFS_HOME=${LFS_HOME}" > config.txt
		echo "LFS_MOUNT=${LFS_MOUNT}" >> config.txt
		echo "LFS_FS=${LFS_FS}" >> config.txt


	else



		FS="$(eval "echo \"$LFS_FS\"")"
		MOUNT="$(eval "echo \"$LFS_MOUNT\"")"

		echo -e "${NEWLINE}${LBLUE}${BOLD}INFO:${DEFAULT} Found root partition ${MOUNT} (${FS}) from config.txt."

		read -n 1 -s -r -p "$(echo -e "${NEWLINE}${YELLOW}${BOLD}[!]${DEFAULT} Do you wish to use this partition?${NEWLINE}1. YES${NEWLINE}0. NO${NEWLINE} ${NEWLINE}")" input

		while [[ $input != "0" && $input != "1" ]]; do
			echo -e "${NEWLINE}${RED}${BOLD}ERROR:${DEFAULT} Not a valid answer!"
			read -n 1 -s -r -p "$(echo -e "${NEWLINE}${YELLOW}${BOLD}[!]${DEFAULT} Do you wish to use this partition?${NEWLINE}1. YES${NEWLINE}0. NO${NEWLINE} ${NEWLINE}")" input

		done

		if (( $input==1 )); then
			sudo mount -v -t $FS $MOUNT $LFS

		else
			mount_root_part

		fi

	fi

	export LFS_MOUNT
	export LFS_FS
	

	echo -e "${NEWLINE}${LBLUE}${BOLD}INFO:${DEFAULT} Checking directory structure."
	setup_directory "${LFS}/sources"

	echo -e "${NEWLINE}${LBLUE}${BOLD}INFO:${DEFAULT} Setting up directory permissions."
	#sudo chown root:root $LFS
	sudo chmod 755 $LFS
	sudo chmod -v a+wt $LFS/sources

	input_break

	echo -e "${NEWLINE}${LBLUE}${BOLD}INFO:${DEFAULT} Downloading packages..."
	download_packages "./wget-list-sysv" "$LFS/sources"

	#sudo chown root:root $LFS/sources/



}

setup_directory(){

	if(($#>0)); then

		if test -d $1; then
			echo -e "$1	${GREEN}${BOLD}[OK]${DEFAULT}"
		else
			echo -e "$1	${YELLOW}${BOLD}[NOT FOUND]${DEFAULT}"
			echo -e "${LBLUE}${BOLD}[Creating directory...]${DEFAULT}"
			mkdir -pv "$1"
		fi

	fi

}

setup_lfs