raw=$( lsblk -o NAME,FSTYPE,MOUNTPOINT )


OIFS= $IFS
IFS="
"

devlist=($raw)

IFS=' ';

devstr=''

NEWLINE=$'\n'

for i in "${devlist[@]}"
do

	#echo $i;

	read -a device <<< $i

	device[0]=$( echo ${device[0]} | sed "s/\W*//" )

	#echo ${device[0]}
	#echo ${device[1]}
	#echo ${device[2]}


	if [[ ${device[1]} != '[SWAP]' && ${device[2]} != "" ]]; then
		
		devstr="${devstr}${NEWLINE}${device[0]}|${device[1]}|${device[2]}"
	
	fi

done

IFS=$OIFS

echo $devstr