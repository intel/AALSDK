import os, re, sys, commands

MQUEUE_MOUNT = "/dev/mqueue/"
SHM_MOUNT = "/dev/shm/"

GLOBAL_LIST = "~/.ase_ipc_global"
USER = os.getenv("USER")

print "############################################################"
print "#                                                          #"
print "#         ASE IPC Cleanup/Sanitation script called         #"
print "#                                                          #"
print "############################################################"


# Check if locations are mounted correctly
print_admin = 0
if (os.path.isdir(MQUEUE_MOUNT) == False):
    print "** WARNING: " + MQUEUE_MOUNT + " location is not mounted in this system. **"
    print_admin = 1
if (os.path.isdir(SHM_MOUNT) == False):
    print "** WARNING: " + SHM_MOUNT + " location is not mounted in this system. **"    
    print_admin = 1

# Ask Admin to mount /dev/{shm,mqueue}
if print_admin == 1:
    print "Please contact your Linux administrator for help."
    print "PLEASE NOTE: ASE will continue to function, but should IPC constructs misbehave, some of the constructs listed in " + GLOBAL_LIST + " will need to be removed manually."

### Straightforward delete operation ###
if print_admin == 0:
    print "IPC mounts seem to be readable... will attempt cleaning up IPC constructs by user '" + USER +"'"
    problem_ipc = commands.getstatusoutput("find /dev/{mqueue,shm}/ -type f -user " + USER )
    if (len(problem_ipc[1])) == 0:
        print "No unlinked IPCs were found.... ALL OK !"
    else:
        ipc_list = problem_ipc[1].split("\n")    
        for ipc in ipc_list:
            os.unlink(ipc)
