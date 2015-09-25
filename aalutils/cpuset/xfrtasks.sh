for var in `cat /sys/fs/cgroup/cpuset/tasks`
do
  sudo /bin/echo $var > /sys/fs/cgroup/cpuset/CpuSubset1/tasks
done
