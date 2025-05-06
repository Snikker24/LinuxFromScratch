#!bin/bash

#Variables

RED='\033[0;31m'
ORANGE='\033[0;33m'
YELLOW='\033[1;93m'
BLUE='\033[0;34m'
LBLUE='\033[0;94m'
GREEN='\033[0;32m'
BLACK='\033[0;30m'

REDBG='\033[30;41m'
GREENBG='\033[30;42m'
LBLUEBG='\e[30;104m'
YELLOWBG='\033[30;103m'


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
				
				devstr="${devstr}${LBLUE}${BOLD}${z}${DEFAULT}	${device[0]}	${device[1]}	${device[2]}	${pttype[$k]}	${device[3]}	${NEWLINE}"

			else

				devstr="${devstr}${GREEN}${BOLD}${devstr}ID	${device[0]}	${device[1]}	${device[2]}	PARTITION-TYPE		${device[3]}	${DEFAULT}${NEWLINE}"
			
			fi
			z=$((z+1))
		fi

		k=$((k+1))

	done


	IFS=$OIFS
	
	echo -e "$devstr"


}

ibreak(){

	local color=$GREENBG

	if [[ (($#>0)) ]]; then
		
		if [[ (($#>1)) ]]; then

			case $1 in

				"-e")
					color=$REDBG
					;;

				"-i")
					color=$LBLUEBG
					;;

				"-w")
					color=$YELLOWBG
					;;


			esac

			echo -e "${NEWLINE}${color}${BOLD}$2${DEFAULT}"

		else

			echo -e "${NEWLINE}${color}${BOLD}$1${DEFAULT}"

		fi

		read -n 1 -s -r

	fi
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

msg(){


	local OIFS=$IFS
	IFS="${NEWLINE}"
	local lines=($(squash $1 $2))
	

	echo -e "${GREEN}${BOLD}[MESSAGE]${DEFAULT}"
	
	for i in ${lines[@]}
	do

		echo -e "${GREEN}${BOLD}[#]${DEFAULT} $i"

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

setup_directory(){

	if(($#>0)); then

		if test -d $1; then
			info 60 "$1: OK"
		else
			warn 60 "$1: NOT FOUND"
			info 60 "Creating directory..."
			mkdir -pv "$1"
		fi

	fi

}

slink(){

	if(($#>1)); then

		if test -L "$2"; then
			info 60 "$2: SYMLINK FOUND"
		else
			warn 60 "$2: SYMLINK MISSING"
			info 60 "Creating symlink..."
			ln -s "$1" "$2"
		fi

	fi

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

	warn 60 "Be careful when choosing root partition! Avoid using host OS partitions as it can break your system!"
	
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

	attr=(${partitions[$id]})

	show_title -b
	#echo -e "$pstr"

	info 40 "Partition /dev/${attr[1]} (${attr[2]} - ${attr[3]}) has been selected as root partition!"
	ibreak "Press any key to mount selected partition..."

	IFS=$OIFS


}

save_vars(){

	local config=""


	if [[ $((${#LFS_HOME}>0)) == 1 ]]; then
		config="LFS_HOME=${LFS_HOME}${NEWLINE}"
	fi

	if [[ $((${#LFS_PART}>0)) == 1 && $((${#LFS_FS}>0)) == 1 ]]; then	
		config="${config}LFS_PART=${LFS_PART}${NEWLINE}"
		config="${config}LFS_FS=${LFS_FS}${NEWLINE}"
	fi

	$(echo "$config" > "config.txt")

}

setup_lfs(){


	show_title

	local config=$(<config.txt)
	
	#echo $config

	local OIFS=$IFS
	IFS=$NEWLINE

	local vars=($config)


	info 40 "Reading variables from config.txt"

	#Reading vars
	for var in ${vars[@]}
	do

		IFS="="
		var=($var)
		IFS=$OIFS

		case ${var[0]} in 
		
			"LFS_HOME")

				LFS_HOME=${var[1]}
				LFS="$(eval "echo \"$LFS_HOME\"")/mnt/lfs"
				;;

			"LFS_PART")

				LFS_PART=${var[1]}
				;;

			"LFS_FS")

				LFS_FS=${var[1]}
				;;

		esac


	done

	echo -e "${GREEN}${BOLD}[ LFS_HOME ]${DEFAULT} $(eval "echo \"$LFS_HOME\"")"
	echo -e "${GREEN}${BOLD}[ LFS ]${DEFAULT} $LFS"
	echo -e "${GREEN}${BOLD}[ LFS_PART ]${DEFAULT} $(eval "echo \"$LFS_PART\"")"
	echo -e "${GREEN}${BOLD}[ LFS_FS ]${DEFAULT} $(eval "echo \"$LFS_FS\"")"

	ibreak -i "Press any key to start setup..."

	show_title -b
	msg 60 "${BOLD}STEP I:${DEFAULT} Please choose the root partition for LFS system."
	ibreak "Press any key to continue..."

	if [[ $LFS_PART != "" && $LFS_FS!="" ]]; then

		local ans

		show_title -b
		info 60 "Found root partition $LFS_PART ($LFS_FS) inside config.txt file. Do you wish to use it? Alternatively, you can choose another one."
		warn 60 "Make sure the partition exists!"
		echo -e "1. Yes${NEWLINE}0. No${NEWLINE}"
		read -n 1 -r -s ans 

		while [[ !$ans=~'^[0-9]+$' && $ans != 0 && $ans != 1 ]]; do

			show_title -b
			errme 40 "Please insert only 0 or 1!"
			info 60 "Found root partition $LFS_PART ($LFS_FS) inside config.txt file. Do you wish to use it? Alternatively, you can choose another one."
			warn 60 "Make sure the partition exists!"
			echo -e "1. Yes${NEWLINE}0. No${NEWLINE}"
			read -n 1 -r -s ans

		done

		if [[ $ans == 0 ]]; then

			select_root_part

		fi

	else

		select_root_part

	fi

	export LFS_HOME
	export LFS
	export LFS_PART
	export LFS_FS

	save_vars
	
	#Mounting root part

	show_title -b
	info 60 "Using $LFS_PART ($LFS_FS) as root partition. Mounting..."
	sudo mount -v -t $LFS_FS $LFS_PART $LFS

	PROGRESS=$((PROGRESS+1))


	#Creating limited directory structure
	
	show_title -b
	msg 80 "${BOLD}STEP II:${DEFAULT} Setting up limited directory structure:"
	
	setup_directory "${LFS}/sources"
	setup_directory "${LFS}/etc"
	setup_directory "${LFS}/var"
	setup_directory "${LFS}/bin"
	setup_directory "${LFS}/sbin"
	setup_directory "${LFS}/lib"
	setup_directory "${LFS}/tools"

	case $(uname -m) in
  	
  		x86_64) setup_directory "${LFS}/lib64" ;;
	
	esac

	ibreak -w "All directories set up! Press any key to continue..."

	#Creating symlinks
	
	show_title -b
	msg 80 "${BOLD}STEP II:${DEFAULT} Setting up limited directory structure:"
	slink "usr/bin" "${LFS}/bin"
	slink "usr/sbin" "${LFS}/sbin"
	slink "usr/lib" "${LFS}/lib"


	ibreak -w "Symlinks configured! Press any key to continue..."


	##LAST STEP
	#Unmounting per user choice

	PROGRESS=100

	show_title -b
	info 60 "Done!"
	msg 60 "Do you wish to unmount root partition ($LFS_PART - $LFS_FS)? Now it is safe to do so."
	echo -e "1. Yes${NEWLINE}0. No${NEWLINE} "
	read -n 1 -r -s ans
	
	while [[ !$ans=~'^[0-9]+$' && $ans != 0 && $ans != 1 ]]; do

		show_title -b
		errme 40 "Please insert only 0 or 1!"
		info 60 "Done!"	
		msg 60 "Do you wish to unmount root partition ($LFS_PART - $LFS_FS)? Now it is safe to do so."
		echo -e "1. Yes${NEWLINE}0. No${NEWLINE} "
		read -n 1 -r -s ans

	done


	if [[ $((ans==1)) ]]; then


		local output=$(sudo umount -v $LFS_PART 2>&1)

		show_title -b
		info 60 "$output. Exiting now..."
		sleep 3

	fi

	clear
}

setup_lfs

