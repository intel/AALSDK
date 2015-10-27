rm pid_list
ps -A -o pid > pid_list
count=0
for var in `cat pid_list`
do
   if [ "$var" == "PID" ]
   then
     continue
   fi
   count=`expr $count + 1`
   echo "Pid: $var Total Pids: $count"
   cat /proc/$var/status | grep PPid:
   cat /proc/$var/status | grep Cpus_allowed_list:
done

