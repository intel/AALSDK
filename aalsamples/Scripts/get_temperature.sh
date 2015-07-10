#!/bin/bash
VENDOR=0x8086
DEVICE=0xbcbc
function conv() {
 res=$(printf "%d" "0x$1")
(( res > 2147483647 )) && (( res -= 4294967295 ))
echo $res 
}

hex_DEFAULT_TEMPERATURE=`echo  "  $(setpci -d $VENDOR:$DEVICE 454.L) "`      >> /dev/null
hex_TEMPERATURE_STATUS=`echo  "  $(setpci -d $VENDOR:$DEVICE 458.L) "`    >> /dev/null
DEFAULT_TEMPERATURE=" $(conv $hex_DEFAULT_TEMPERATURE)"
TEMPERATURE_STATUS="$(conv $hex_TEMPERATURE_STATUS)"
if [ "$DEFAULT_TEMPERATURE" -gt "256" ];then
        DEFAULT_TEMPERATURE=$(expr $DEFAULT_TEMPERATURE - 256)
        echo "Temperature sensing enabled"
        echo "Default temperature threshold of the machine "$DEFAULT_TEMPERATURE $'\xc2\xb0'C
        echo "Current temperature of the machine" $TEMPERATURE_STATUS $'\xc2\xb0'C
        else
        echo "Temperature sensing not enabled"
        echo "Default temperature threshold of the machine "$DEFAULT_TEMPERATURE $'\xc2\xb0'C
        echo -e "To enable temperature sensing press yes/no ? "
        read word
        if [ $word = yes ];then
                DEFAULT_TEMPERATURE=$(expr $DEFAULT_TEMPERATURE + 256)
                hex_DEFAULT_TEMPERATURE=`printf '%x\n' $DEFAULT_TEMPERATURE`
                setpci -d $VENDOR:$DEVICE 454.L=$hex_DEFAULT_TEMPERATURE
                ./get_temp.sh
                else
                exit
        fi
fi

