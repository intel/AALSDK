==============================================================================================================
Getting started (Using valapps.py)
==============================================================================================================
1. If the AALSDK source hasn't been built, build AALSDK source:
   Use valapps.py with the -b option to build the sdk source. 
   ./valapps.py -b --sdksrc /path/to/sdk/src

2. Load the kernel driver (ONLY if built with step 1 above)
   valapps.py may be used to load the driver ONLY if valapps.py was used to build the binaries (step 1 above).
   This will call "sudo <aalkernel build>/insmod cci" 
   ./valapps.py -L


3. Build valapps suite of tests:
   Use valapps.py with the -M option to build the tests against aalsdk libraries.
   A. If the AALSDK binaries have been previously built:
      ./valapps.py -M --aalsdk /path/to/sdk/install/prefix
   B. If the AALSDK binaries were built using step 1 above:
      ./valapps.py -M

4. Create rtl.json file (rtl.json file is used by the tests to locate any necessary rtl collateral).
   The current rtl files are .rbf files that make up the NLB (both modes 0 and mode 3).
   ./valapps.py --rtl /path/to/rtl/direcytory


5. Run tets:
   Use valapps.py with the -t option.
   ./valapps.py -t
   or 
   ./valapps.py -t -r <test suite>


valapps.py is a script that can do it all for valapps. It can:
    build AALSDK in the same codebase
    build valapps using either AALSDK it built or an optional path to an existing aalsdk distribution.
    execute the valapps wrapped as python unittest.TestCase objects. 
        *** For tests to be discovered by valapps.py, they should be defined in an __init__.py file in the 
        *** directory of the valapp. 
        *** Details for this are found below 


Below is the usage information for running valapps.py
usage: valapps.py [-h] [-k AALSDK] [-t] [-m [MAKE_TESTS [MAKE_TESTS ...]]]
                  [-M] [-b] [-S SDKSRC] [-c] [-r [RUN_TESTS [RUN_TESTS ...]]]
                  [-L] [-d {cci,cci-sim}] [-l] [-R RTL]
                  [--consolidate-results CONSOLIDATE_RESULTS] [--help-full]

optional arguments:
  -h, --help            show this help message and exit
  -k AALSDK, --aalsdk AALSDK
                        Location of aalsdk installation to link with. If not
                        specified, this script will build both
                        aaluser/aalkernel from this codebase (Assuming that
                        this directory is parallel to the aforementioned
                        directories).
  -t, --test-only       Run tests only, don't try to rebuild valapps
  -m [MAKE_TESTS [MAKE_TESTS ...]], --make-tests [MAKE_TESTS [MAKE_TESTS ...]]
                        Make only the tests in the list
  -M, --make-only       Make only the tests. Do not run tests
  -b, --build-sdk       Build sdk either in this code base or that pointed to
                        by --sdksrc
  -S SDKSRC, --sdksrc SDKSRC
                        root directory containing aalsdk source code
  -c, --clean-only      Run 'make clean' on the tests. Do not run tests
  -r [RUN_TESTS [RUN_TESTS ...]], --run-tests [RUN_TESTS [RUN_TESTS ...]]
                        Run tests matching pattern(s)
  -L, --load-driver     Load kernel driver from kernel build
  -d {cci,cci-sim}, --driver {cci,cci-sim}
                        Which driver to load (if loading driver after build).
                        Default is cci-sim
  -l, --list            Lists tests discovered and the documentation (if
                        found) for each test.
  -R RTL, --rtl RTL     Path to directory containing RTL files (for NLB *.rbf
                        files). Look for *0.rbf and *3.rbf files in this
                        directory and cache them in rtl.py. NOTE: If more than
                        one file exists for each wildcard (*0.rbf and *3.rbf)
                        in the given path, this script will cache the path to
                        the first one found.
  --consolidate-results CONSOLIDATE_RESULTS
                        Consolidate all results to one file (specified by this
                        argument)
  --help-full           shows detailed help information gernated from pydoc
                        strings in this file


==============================================================================================================
More info (Developing valapps)
==============================================================================================================
At the heart of valapps.py is the test_suite class. Below is the docstring from the class:
  test_suite is used to generate testcase class compatible with the built-in python unitttest module.
  An instance of this class can "discover" tests in the immediate sub-directories by looking for the __init__.py file.
  If found, the directory will be imported into the namespace after which the test_suite instance will look for
  any methods that begin with the word 'test'. If any methods are found in the newly imported module, a test case class
  will be dynamically generated which will derive both from unittest.TestCase and this class. This will give the newly created
  class access to members inherited from the TestCase class as well as some convenience methods implemented in the test_suite class.
  For example, if a test method is defined in an __init__.py file as the following:
 
      def test_SampleTest(self):
          output = self.stdout('./executable arg1 arg2 ...')
          self.assertTrue(len(output) > 42)
 
  'self' refers to an instance of the newly created class that inherits from both unittest.TestCase as well as this class, test_suite.
  This is why it can call self.stdout to execute a process and capture its stdout as well as call self.assertTrue (which comes from TestCase).


