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

STEPS=5

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

		if test -L "$2" || test -e "$2"; then
			info 60 "$2: SYMLINK FOUND"
		else
			warn 60 "$2: SYMLINK MISSING"
			info 60 "Creating symlink..."
			sudo ln -sv "$1" "$2"
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

download_packages(){

	local OIFS=$IFS

	IFS=$NEWLINE

	show_title -b


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
		info 70 "Downloading package:  ${BOLD}${ORANGE}$packname${DEFAULT} ${BOLD}${LBLUE}[$i/$len]${DEFAULT}"
		wget -q -P "$target_dir/" $pack
		info 70 "${BOLD}${GREEN}[DONE]${DEFAULT}"

		i=$((i+1))

	done

}

pkgdl(){



	if [[ $#>1 ]]; then


		if test -d "$2"; then
		
			wget -q -P "$2/" "$1"
		
		else

			errme 60 "Location $2 doesn't exist or is not a directory!"
		fi

	else


		sudo wget -q -P "$1"


	fi

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
	setup_directory "${LFS}/tools"
	setup_directory "${LFS}/usr"
	setup_directory "${LFS}/usr/bin"
	setup_directory "${LFS}/usr/sbin"
	setup_directory "${LFS}/usr/lib"
	

	case $(uname -m) in
  	
  		x86_64) setup_directory "${LFS}/lib64" ;;
	
	esac

	ibreak -w "All directories set up! Press any key to continue..."

	#Creating symlinks
	
	show_title -b
	msg 80 "${BOLD}STEP II:${DEFAULT} Setting up limited directory structure:"
	slink "/usr/bin" "${LFS}"
	slink "/usr/sbin" "${LFS}"
	slink "/usr/lib" "${LFS}"


	ibreak -w "Symlinks configured! Press any key to continue..."


	#Downloading packages
	show_title -b
	msg 80 "${BOLD}STEP III:${DEFAULT} Downloading packages..."

	ibreak -i "Press any key to start download..."


	IFS=$NEWLINE

	local packages=$(<"./wget-list-sysv")
	packages=($packages)

	
	local pkcnt=${#packages[@]}
	local pki=1

	local pklog=""
	for pack in ${packages[@]}
	do

		show_title -b
		msg 80 "${BOLD}STEP III:${DEFAULT} Downloading packages..."

		IFS="/"
		local packname=($pack)
		packname=${packname[$((${#packname[@]}-1))]}
		IFS=$OIFS
		
		local prompt="Downloading package [$pki/$pkcnt]: $packname"

		echo "$pklog"
		info 70 "$prompt"

		local res

		if test -f "${LFS}/sources/$packname"; then

			res=$(echo -e "${GREEN}${BOLD}[ALREADY EXISTS]${DEFAULT}")

		else

			res=$(pkgdl "$pack" "${LFS}/sources")


			if [[ $res == "" ]]; then
				
				res=$(echo -e "${YELLOW}${BOLD}[DONE]${DEFAULT}")

			fi

		fi

		pklog="$pklog${NEWLINE}$(info 70 "$prompt")${NEWLINE}$res"


		show_title -b
		msg 80 "${BOLD}STEP III:${DEFAULT} Downloading packages..."
		echo -e "$pklog"
		pki=$((pki+1))


	done

	msg 60 "Download complete!"
	ibreak -i "Press any key to continue..."

	##Creating lfs user

	show_title -b
	msg 80 "${BOLD}STEP IV:${DEFAULT} Creating LFS user:"
	warn 60 "You are about to create a new user profile with the name \"lfs\" (can be changed later via \"usermod -l <new_username>\" command)"
	warn 60 "If you wish to change the name later don't forget to change the home directory as well using \"usermod -d <new_home> -m <new_username>\" command)"
	ibreak -w "Press any key to continue..."

	show_title -b
	msg 80 "${BOLD}STEP IV:${DEFAULT} Creating LFS user:"
	
	info 60 "Creating user group: lfs"
	sudo groupadd lfs
	echo -e "${YELLOW}${BOLD}[DONE]${DEFAULT}"

	info 60 "Creating user: lfs"
	sudo useradd -s "/bin/bash" -g "lfs" -m -k "/dev/null" lfs
	echo -e "${YELLOW}${BOLD}[DONE]${DEFAULT}"

	info 60 "Please create a password for new user: lfs"
	sudo passwd lfs
	echo -e "${YELLOW}${BOLD}[DONE]${DEFAULT}"

	ibreak -i "Press any key to continue..."


	##Filesystem permissions
	
	show_title -b
	msg 80 "${BOLD}STEP V:${DEFAULT} Setting up file permissions:"

	info 60 "Creating permissions for LFS directory structure:"
	msg 60 "${LFS}"
	chown root:root "${LFS}"
	chmod 755 "${LFS}"
	echo -e "${YELLOW}${BOLD}[DONE]${DEFAULT}"

	msg 60 "${LFS}/sources"
	chmod -v a+wt "${LFS}/sources"
	chown root:root "${LFS}/sources/*"
	echo -e "${YELLOW}${BOLD}[DONE]${DEFAULT}"

	msg 60 "${LFS}/usr"
	chown -v lfs "${LFS}/usr"
	chown -v lfs "${LFS}/usr/*"
	echo -e "${YELLOW}${BOLD}[DONE]${DEFAULT}"
	
	msg 60 "${LFS}/var"
	chown -v lfs "${LFS}/var"
	echo -e "${YELLOW}${BOLD}[DONE]${DEFAULT}"

	msg 60 "${LFS}/etc"
	chown -v lfs "${LFS}/etc"
	echo -e "${YELLOW}${BOLD}[DONE]${DEFAULT}"
	
	msg 60 "${LFS}/tools"
	chown -v lfs "${LFS}/tools"
	echo -e "${YELLOW}${BOLD}[DONE]${DEFAULT}"

	### Changing users
	show_title -b
	warn 80 "From this point on all commands will be executed as user \"lfs\"."
	ibreak -w "Press any key to continue..."
	info 80 "Please login as user \"lfs\":"
	#su - lfs
	ibreak -w "Press any key to continue..."


	##LFS bash setup
	
	show_title -b
	msg 80 "${BOLD}STEP VI:${DEFAULT} Setting up bash scripts for \"lfs\" user:"

	usr_home="$(sudo -u lfs -H bash -c "echo \$HOME")"

	info 70 "Creating bash profile in: $usr_home/.bash_profile"
	cmdline="exec env -i HOME=$usr_home TERM=$TERM PS1='\u:\w\\$ ' /bin/bash"
	sudo -u lfs -H bash -c "printf $cmdline > /\$HOME/.bash_profile"
	echo -e "${YELLOW}${BOLD}[DONE]${DEFAULT}"

	info 70 "Creating \".bashrc\" in: $usr_home/.bashrc"

	cores="$(nproc)"
	if [ $cores = "" ]; then
		cores=1
	fi

	local cmdline="set +h$"
	cmdline="$cmdline${NEWLINE}umask 022"
	cmdline="$cmdline${NEWLINE}LFS=$LFS"
	cmdline="$cmdline${NEWLINE}LC_ALL=POSIX"
	cmdline="$cmdline${NEWLINE}LFS_TGT=$(uname -m)-lfs-linux-gnu"
	cmdline="$cmdline${NEWLINE}PATH=/usr/bin"
	cmdline="$cmdline${NEWLINE}if [ ! -L /bin ]; then PATH=/bin:\$PATH; fi"
	cmdline="$cmdline${NEWLINE}PATH=\$LFS/tools/bin:\$PATH"
	cmdline="$cmdline${NEWLINE}CONFIG_SITE=$LFS/usr/share/config.site"
	cmdline="$cmdline${NEWLINE}export LFS LC_ALL LFS_TGT PATH CONFIG_SITE"
	cmdline="$cmdline${NEWLINE}export MAKEFLAGS=-j$cores"

	printf "Here is the script:${NEWLINE}$cmdline"
	sudo -u lfs -H bash -c "printf \"$cmdline\" > /$usr_home/.bashrc"
	content=$(sudo -u lfs -H bash -c "cat /$usr_home/.bashrc")
	echo $content
	echo -e "${YELLOW}${BOLD}[DONE]${DEFAULT}"

	info 60 "Forcing bash to use .bash_profile"
	sudo -u lfs -H bash -c "source /$usr_home/.bash_profile"
	echo -e "${YELLOW}${BOLD}[DONE]${DEFAULT}"

	ibreak -i "Press any key to continue..."


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


	if [[ $((ans==1)) == 1 ]]; then


		local output=$(sudo umount -v $LFS_PART 2>&1)

		show_title -b
		info 60 "$output. Exiting now..."
		sleep 3

	else

		show_title -b
		info 60 "Exiting now..."
		sleep 3

	fi

	clear
}

setup_lfs