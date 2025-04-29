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

#Functions

show_title(){

	clear

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

warn(){


	local OIFS=$IFS

	
	size=${#${$1//[^[:alnum:]]/}}

	IFS=" "

	count=0

	


	IFS=$OIFS

}



select_root_part(){


	local partitions="$(show_part_table)"
	echo -e "$partitions"

	local OIFS=$IFS	

	partitions=($partitions)

	count=${partitions[@]}

	read -n 1 -p "$(echo -e "${YELLOW}${BOLD}[!]${DEFAULT} Please enter root partition ID:")"

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
select_root_part


