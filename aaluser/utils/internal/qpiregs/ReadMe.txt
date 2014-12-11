qpiregs is a simple utility that reads & prints a small set of QPI registers.

The application is linked with libpci, a part of the pciutils-devel package.
Be sure that the pciutils-devel package (and GNU make, gcc/g++) is installed on the system prior to building the application.

To install pciutils-devel on openSUSE:
$ sudo zypper in pciutils-devel

To build qpiregs:
$ cd AALSDK/utils/internal/qpiregs # this directory
$ make

To run qpiregs:
$ ./qpiregs

