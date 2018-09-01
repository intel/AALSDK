# AALSDK
To build anything, first start with prep-build:

$ ./prep-build

To build aaluser

$ cd aaluser
$ mkdir mybuild
$ cd mybuild
$ ../configure
$ make
$ sudo make install

**NOTE:** to build a local version of aaluser that will install locally for debugging,
there are several techniques, but this one definitely works:

$ cd aaluser
$ mkdir mybuild
$ cd mybuild
$ ../configure --prefix=$PWD/../../myinst   # myinst is the target install directory
$ make -j4                                  # more threads, faster build
$ make install                              # no sudo, because going to a local directory


## To build aalsamples (which depend upon aaluser installation, so need that directory)
*assume you are still in aaluser/mybuild*

$ cd ../../aalsamples
$ make prefix=$PWD/../myinst                # note this is not --prefix, but prefix
                                            # point to aaluser installation directory tree
## To execute a sample, e.g. HelloAAL ...
*assume you are still in aalsamples*

$ source ld_library_path                    # add pointer to AAL Ping Service in Hello_AAL_Service
$ Hello_AAL/SW/helloaal                     # execute AAL Ping Client against the AAL Ping Service


## To build aalkernel (good up through about kernel 3.6)

*assume you are in aalsdk*

$ cd aalkernel
$ mkdir mybuild
$ cd mybuild
$ ../configure --prefix=$PWD/../../myinst   # same local directory as AAL, but could be anywhere
$ make -j4
$ make install

## To load the driver (note that the HW likely no longer exists)

$ cd $PWD/../../myinst
$ ./insdrv
