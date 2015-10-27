ps -A -o pid > pid_lx
for var in `cat pid_lx`
do
  sudo /bin/echo $var > /sys/fs/cgroup/cpuset/CpuSubset1/tasks
done
rm pid_lx
