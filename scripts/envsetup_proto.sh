#!bin/bash

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

PROGRESS=0

#Functions

show_title(){

	clear

	echo -e "${LBLUE}${BOLD}"
	
	
	echo -e "╔════════════════════════════════════╗"
	echo -e "║ _     _                            ║"
	echo -e "║| |   (_)_ __  _   ___  __          ║"
	echo -e "║| |   | | '_ \| | | \ \/ /          ║"
	echo -e "║| |___| | | | | |_| |>  <           ║"
	echo -e "║|_____|_|_| |_|\__,_/_/\_\          ║"
	echo -e "║|  ___| __ ___  _ __ ___            ║"
	echo -e "║| |_ | '__/ _ \| '_ \` _ \           ║"
	echo -e "║|  _|| | | (_) | | | | | |          ║"
	echo -e "║|_|__|_|  \___/|_| |_|_|_|    _     ║"
	echo -e "║/ ___|  ___ _ __ __ _| |_ ___| |__  ║"
	echo -e "║\___ \ / __| '__/ _\` | __/ __| '_ \ ║"
	echo -e "║ ___) | (__| | | (_| | || (__| | | |║"
	echo -e "║|____/ \___|_|  \__,_|\__\___|_| |_|║"
	echo -e "╚════════════════════════════════════╝"

	echo -e "${GREEN}${BOLD}> LinuxFromScratch setup utility script${DEFAULT}${NEWLINE}"


	if [[ (($#>0)) && $1=="-b" ]]; then
		percentage_bar $PROGRESS 60
	fi
}


show_part_table(){

	local OIFS=$IFS
	IFS=$NEWLINE

	local devices=($( lsblk -o NAME,SIZE,FSTYPE,MOUNTPOINT))
	local pttype=($(lsblk -o PARTN,PARTTYPENAME))

	local k=0

	for i in ${pttype[@]}
	do
		#pttype[$z]=${pttype[$z]:1}
		pttype[$k]=${i//[^[:ascii:][:space:]]/}
		pttype[$k]=${pttype[$k]:6}
		k=$((k+1))
	done

	k=0
	local devstr=""

	local z=0


	IFS=' '

	for device in "${devices[@]}"
	do
		#echo $i;

		#device=($device)
		read -a device <<< $device


		#device[0]=$( echo ${device[0]} | sed "s/\W*//" )
		device[0]=${device[0]//[^[:alnum:]]/}


		if [[ ${pttype[$k]} != '' && ${pttype[$k]} != "EFI System" && ${pttype[$k]} != "Linux swap" && ${device[2]} != "" ]]; then

			if (( $k != 0 )); then
				
				devstr="${devstr}${LBLUE}${BOLD}${z}${DEFAULT}	${device[0]}	${device[1]}	${device[2]}	${device[3]}		${pttype[$k]}${NEWLINE}"

			else

				devstr="${devstr}${GREEN}${BOLD}${devstr}ID	${device[0]}	${device[1]}	${device[2]}	${device[3]}	PARTITION-TYPE${DEFAULT}${NEWLINE}"
			
			fi
			z=$((z+1))
		fi

		k=$((k+1))

	done


	IFS=$OIFS
	
	echo -e "$devstr"


}

input_break(){
	echo -e "${NEWLINE}${GREENBG}${BLACk}${BOLD}Press any key to continue...${DEFAULT}"
	read -n 1 -s -r
}

squash(){

	local OIFS=$IFS
	
	local size=$2
	size=${size//[^[:alnum:]]/}
	size=${#size}

	local count=0

	IFS=" ${NEWLINE}"
	local words=($2)

	local x=0
	local m=$1
	

	#echo -en "${YELLOW}${BOLD}[WARNING]${DEFAULT}"
	
	for i in ${words[@]}
	do


		if [[ $((count+${#i}+1 < m*x)) == 1 ]] ; then

			echo -en " $i"

			count=$((count+1+${#i}))

		else

			#echo -en "${NEWLINE}${YELLOW}${BOLD}[#]${DEFAULT} "
			echo -en "${NEWLINE}$i"

			#while [[ $(( count < m*x )) == 1 ]]; do

				#echo -en " "
				#count=$((count+1))

			#done


			count=$((count+${#i}))
			x=$((x+1))
			m=$((m*x))

		fi


	done

	echo -en "${NEWLINE}"

	IFS=$OIFS

}

warn(){


	local OIFS=$IFS
	IFS="${NEWLINE}"
	local lines=($(squash $1 $2))
	

	echo -e "${YELLOW}${BOLD}[WARNING]${DEFAULT}"
	
	for i in ${lines[@]}
	do

		echo -e "${YELLOW}${BOLD}[#]${DEFAULT} $i"

	done

	IFS=$OIFS

}

errme(){


	local OIFS=$IFS
	IFS="${NEWLINE}"
	local lines=($(squash $1 $2))
	

	echo -e "${RED}${BOLD}[ERROR]${DEFAULT}"
	
	for i in ${lines[@]}
	do

		echo -e "${RED}${BOLD}[#]${DEFAULT} $i"

	done

	IFS=$OIFS

}

info(){


	local OIFS=$IFS
	IFS="${NEWLINE}"
	local lines=($(squash $1 $2))
	

	echo -e "${LBLUE}${BOLD}[INFO]${DEFAULT}"
	
	for i in ${lines[@]}
	do

		echo -e "${LBLUE}${BOLD}[#]${DEFAULT} $i"

	done

	IFS=$OIFS

}


percentage_bar(){

	local percent=$1
	local len=$2

	echo -en "[${GREEN}${BOLD}"

	fill=$((percent*len/100))

	for ((i=1; i<=len; i++))
	do

		if [[ $((i <= fill)) == 1 ]]; then

			echo -en "■"

		else

			echo -en "□"

		fi

	done

	echo -en "${DEFAULT}]${percent}%${NEWLINE}"



}


setup_lfs_home(){


	local config=$(<config.txt)
	
	local OIFS=$IFS
	IFS=$NEWLINE

	local vars=($config)

	IFS="="

	info 40 "Generating variables from config.txt"

	for var in ${vars[@]}
	do

		var=($var)
		case ${varmap[0]} in 
		
			"LFS_HOME")

				LFS_HOME=${varmap[1]}
				LFS="$(eval "echo \"$LFS_HOME\"")/mnt/lfs"
				echo -e "${GREEN}${BOLD}[${varmap[0]}]${DEFAULT} $(eval "echo \"$LFS_HOME\"")"
				echo -e "${GREEN}${BOLD}[LFS]${DEFAULT} $LFS"
				;;

			"LFS_PART")

				LFS_MOUNT=${varmap[1]}
				echo -e "${GREEN}${BOLD}[${varmap[0]}]${DEFAULT} $(eval "echo \"$LFS_PART\"")"
				;;

			"LFS_FS")

				LFS_FS=${varmap[1]}
				echo -e "${GREEN}${BOLD}[${varmap[0]}]${DEFAULT} $(eval "echo \"$LFS_FS\"")"
				;;

		esac


	done

	input_break

}


save_vars(){

	echo

}

#Root partition select+mount

select_root_part(){

	show_title -b
	

	local pstr="$(show_part_table)"

	local partitions

	local OIFS=$IFS	

	IFS="${NEWLINE}"

	partitions=($pstr)

	count=${#partitions[@]}

	IFS=$OIFS

	warn 40 "Be careful when choosing root partition! Avoid using host OS partitions as it can break your system!"
	
	info 40 "Suitable devices found: $((count-1))"

	echo -e "$pstr"

	local id
	read -p "$(echo -e "${YELLOW}${BOLD}[!]${DEFAULT} Please enter root partition ID:")" id
	echo ""


	while [[ !$id=~'^[0-9]+$' && $id > $((count-1)) || $id <1 ]]; do

		show_title -b
		warn 40 "Be careful when choosing root partition! Avoid using host OS partitions as it can break your system!"

		info 40 "Suitable devices found: $((count-1))"
		echo -e "$pstr"

		errme 60 "Not a valid partition! Try again..."
		read  -p "$(echo -e "${YELLOW}${BOLD}[!]${DEFAULT} Please enter root partition ID:")" id
		echo ""

	done


	IFS="	"

	PROGRESS=$((PROGRESS+1))

	attr=(${partitions[$id]})

	show_title -b
	#echo -e "$pstr"

	info 40 "Partition $id ( /dev/${attr[1]} | ${attr[2]} | ${attr[3]} ) has been selected as root partition!"
	info 40 "Mounting selected partition..."

	export LFS_FS=${attr[3]}
	export LFS_PART=${attr[1]}
	sudo mount -v -t $LFS_FS $LFS_PART $LFS


	IFS=$OIFS


}

select_root_part0(){


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

	local devlist=($( lsblk -o NAME,SIZE,FSTYPE,MOUNTPOINT))

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
					devstr="${devstr}${LBLUE}${BOLD}${k}${DEFAULT}	${device[0]}	${device[1]}	${device[2]}	${pttype[$k]}		${device[3]}${NEWLINE}"
				#else
					#devstr="${devstr}${k} ${device[0]} ${device[1]} ${device[2]}"
				#fi

			else
				devstr="${GREEN}${BOLD}${devstr}ID	${device[0]}	${device[1]}	${device[2]}	PARTITION-TYPE		${device[3]}${DEFAULT}${NEWLINE}"
				
			fi

		fi

		k=$((k+1));

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

		fi

	else

		echo -e "${RED}${BOLD}ERROR:${DEFAULT} No suitable partitions found!"
		echo -e "${RED}${BOLD}ERROR:${DEFAULT} Please create a new partition and run the script again."
		echo -e "${RED}${BOLD}ERROR:${DEFAULT} Exiting..."
		exit
	fi

	IFS=$OIFS

}

#show_part_table
setup_lfs_home
select_root_part


