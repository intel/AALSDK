PackagePwrLimit1=$(rdmsr -c0 -p 0 0x610)
echo "Raw Power Package Limits Msr: " $PackagePwrLimit1
Mask=0x0000000000007FFF
echo "Masking with: " $Mask
Limit1Val=$(( PackagePwrLimit1 & Mask ))
echo "Power Limit 1  " $Limit1Val
PackagePwrUnit=$(rdmsr -c0 -p 0 0x606)
echo $PackagePwrUnit
UnitVal=$(( PackagePwrUnit & 0x000000000000000F ))
echo "Exponent for units: " $UnitVal
UnitVal=$(echo "2^$UnitVal" | bc )
echo "Divsor of Raw Limit1: " $UnitVal
TotalWatts=$(echo "$Limit1Val / $UnitVal" | bc )
echo "Total Watts Allowed in Package" $TotalWatts
FpgaWatts=$1
echo "Fpga Watts needed: " $FpgaWatts
AvailWatts=$(echo "$TotalWatts - $FpgaWatts" | bc)
echo "Watts available for Cores: " $AvailWatts
MaxCoresAllowed=$(echo "$AvailWatts / 5" | bc)
echo "Cores allowed at 5 watts per core: " $MaxCoresAllowed
MaxCpus=$(echo "$MaxCoresAllowed * 2" | bc)
echo "Threads allowed: " $MaxCpus
MaxNumberForTaskSet=$(echo "$MaxCpus - 1" | bc)
echo "Max number in mask input to taskset: " $MaxNumberForTaskSet
TaskMaskStart="0-"
TaskSetMask=$TaskMaskStart$MaxNumberForTaskSet
echo "Taskset cpu mask: " $TaskSetMask
ps -A -o pid > pid_lx1
for var in `cat pid_lx1`
do
  sudo taskset -pc $TaskSetMask $var
done
rm pid_lx1
