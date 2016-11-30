# valapps test development and execution framework

## valapps
valapps is a set of validation tests developed to exercise AAL on real hardware.
The scope of valapps is to test and validate that certain functional [requirements](testplan.html) are met.
valapps can be broken down into the following components:

1.  The build/development environment. This is covered by [buildenv.py](#buildenv). 
2.  The development APIs used for AAL and assertions. This is covered by
  *  The __waal__ libraries (*wafu, wutils, aalcif, and aal.py*) in the [valapp](valapp/readme.html) directory
  *  The [google test](https://github.com/google/googletest) framework
3. The test execution environment. This is covered by [runit.py](#runit)

<a name="buildenv"/>
## Build Environment
The build environment can be provisioned using the buildenv.py script which is used to:

1.  Download/build/install dependencies.
2.  Build AAL SDK (both aaluser and aalkernel)
3.  Build the valapps binaries and install

### Dependency Resolution (bootstrapping)
Dependencies can and should be used from packages (git clones, compressed files like tar.gz or .zip files, deb/rpm files, etc.).
Dependency resolution involves several steps:

1.  Fetching (or downloading)
2.  Unpacking (if needed)
3.  Building/installing 

In order for buildenv.py to process dependencies, they should be placed in a dependency manifest file (typically referred to as packages.json) in JSON format.
Each dependency specification includes the fetcher, an optional unpacker, and a builder. Each of these items may or may not require additional parameters that will be used at runtime as each dependency is processed. 
An example package in a packages.json file can look like this:

    {
        "name": "cmake",
        "version" : "3.7.0",
        "type" : "executable",
        "install" : ".develop",
        "url" : "https://cmake.org/files/v3.7/cmake-3.7.0-rc2.tar.gz",
        "fetcher" :
        {
            "name": "urlfetcher",
            "file" : "cmake-3.7.0-rc2.tar.gz"
        },
        "unpacker" : "tar",
        "installer" :
        {
            "name": "make_installer"
        }
    }

The preceding example tells buildenv.py to download cmake from the given url, unpack it using the tar unpacker, then install using conventional autotools generated makefiles. The `"install" : ".develop"` section is an indication to buildenv.py to install this in a directory outside the default installation directory of `.packages`.

To bootstrap packages using buildenv.py, use the `-B` or `--bootstrap` command line argument.
Some example include:
    
    >./buildenv.py -B  # processes packages in a file called packages.json 

    >./buildenv.py -B -p other-packages.json # processes packages in a file called other-packages.json

    >./buildenv.py -B -P cmake # processes only the cmake package in packages.json

### Building AAL SDK (Optional)
One may use buildenv.py to build and install AAL SDK given the source code. As valapps currently is part of the AAL SDK source tree in its git repository, one may point to the root of the git clone as the source directory of AAL.
The following example builds AAL SDK one directory above the current directory:
    >./buildenv.py -b -s ../
    
NOTE: Building AAL SDK with buildenv.py will install its binaries in the default installation directory of `.packages`.

### Building valapps 
Finally, to build valapps, use the `-M or --make` command line arguments. This can also use AAL SDK binaries built somewhere else when linking valapps.
__This will not install AAL SDK binaries in the default installation directory of `.packages`.__ 

valapps uses CMake for its build system. Following cmake build convention, buildenv.py will make a `build` directory where all binaries will be built. When buildenv.py configures cmake, it will set the installation prefix to `.packages`. The default build configuration for valapps is Release, however, it may be changed to Debug using the `-C or --configuration` command line argument. 

buildenv.py can also be used to create distribution packages of the built binaries using the `-K or --make-package` command line argument. A typical filename of a distribution package will be valapps-<git hash>.tgz.

Examples of using buildenv.py to build valapps include:

    >./buildenv.py -M # this builds and installs valapps to .packages

    >./buildenv.py -M -K # builds/install valapps, then makes a distribution package in default format (tar.gz)

    >./buildenv.py -M -K -G RPM # builds/install valapps, then makes an RPM package

To get more detailed help for running buildenv.py, use the `-h or --help` command line argument.

## Development APIs
Because valapps uses APIs in C++11, it is a requirement to use a compiler that supports the C++11 standard (or greater). 
The only other requirement is use of the [JSONCPP](https://github.com/open-source-parsers/jsoncpp) library for parsing configuration files. 
For convenience, the JSONCPP source files are included with the valapps code base under the valapp directory. These files are:

* json/json.h
* json/json-forward.h
* json.cpp (an amalgamation of all the JSONCPP source files and compiled into the jsoncpp library)

### The wafu Library
The wafu library is a thin wrapper around the AALSDK and encapsulates common client side APIs including callbacks required for interfacing with the AAL runtime. 
Motivation for creating the wafu library stems from the:
* The desire to have a simplified API for developing AAL tests
* Eliminating boiler-plate code used when developing AAL tests
* A flexible and dynamic framework that can have service parameters defined in text files

The two most important components of the wafu library are the service_manager and the service_client.

#### service_manager
The service_manager class  derives from the AAL class CAASBase and implements the AAL interface class IRuntimeClient. 
As a [singleton](https://en.wikipedia.org/wiki/Singleton_pattern), it serves as the entry point into an AAL application. 
In other words, there will be only one instance of this class in any application that uses it. Conceptually, the two most important functions that the service_manager 
provides are the service_manager::define_services(const std::string &configFile) and service_manager::create_service(const std::string &service_name). The former is used to 
define service parameters which will be stored internally (using AAL data structures) by the service_manager and the latter uses those parameters to call AAL functions to 
create a given service (returning an object instance that derives from service_client). A service_client derived instance is created by the service_manager whenever the 
service_manager::create_service method is invoked. This derived class is determined by the JSON input file by the client_type field and must be registered with the service_manager 
before service_manager::create_service is called. If no client_type is given, an object of type afu_client will be used when creating the service_client object. The valapps
codebase includes a standard services input file, called [services.json](services.html).

#### service_client
The service_client implements the AAL::IService_Client interface class and because it derives from AAL::CAASBase, it can be used to get interface objects to an AAL service.  
As a convenience, one may optionally register interfaces with their corresponding identifiers (of type AAL::btIID) to be later retrieved using only the interface type.
A typical use case of this is for a service developer to create a class which derives from service_client to serve as the client class for that service. The developer may
then use the service_client::register_interface<T>(AAL::btIID serviceName) method to associate the identifier with the class type. A user of the service may then only use
the service_client::get_service() method to get an instance of the interface that has registered in the constructor of the concrete service client class.
An excellent example of this usage can be seen by looking at the afu_client that is part of the wafu library. 
This class registers common interface classes used by the ALI service and can be used to  interface with most green bitstreams using ALI interfaces such as IALIMMIO.

### The google test valapps
The wafu library was originally developed independent of google test and as such included its own test execution engine. As the codebase evolved and integrated the google test
framework for not only test development but for test execution, many of the classes/functions in the original wafu library have become unnecessary and will be deprecated.
Two classes that are still being used for valapps development/execution are test_manager and test_context.

#### test_manager
The valapps framework is designed such that tests are developed using google test APIs and compiled in test libraries. The test_manager is also a singleton and its primary function is to
load the test libraries. As test libraries are loaded into the runtime environment, the tests will be registered with the google test runtime allowing for their 
discovery and execution. The test_manager is also used to register command line arguments that may be used by a test fixture.


#### test_context
It is recommended to develop valapps tests as google test fixtures that inherit from a google test class (::testing::Test or ::testing::TestWithParam<>) and a test_context.
Deriving from test_context will allow the test implementation to call its inherited method, get_service, to get a shared_ptr to a service_client object. 
Using this interface will also ensure that service allocation is complete by the AAL runtime before returning the service_client object.
Test implementations should then use the [test_context::get_service<T>(const std::string &service_name)](@ref test_context) method to get a shared_ptr to a service_client object.
Another important function defined in the test_context is the test_context::args() method. This is used to get a reference to the raw test arguments registered in the 
main function and parse them.

<a name="main" />
### valapps main
The main function for valapps tests is found in the valapps codebase under gtest/main.cpp. The logic contained in here can be broken down into the following steps:

1. Parses the arguments the first time.
   This gets the service specification file (defaults to services.json if not given) and the test library.
   Any arguments after the `--` literal will be used by the google test runtime and the optionally the test fixture.
2. Load the test library (given by the -t|--testlib argument).
   As the library is loaded, the google tests in the library will be discoverable by the google test runtime.
3. Start the wafu service_manager.
4. Call InitGoogleTest with the remaining arguments. Any non google test arguments will be left in argv and registered with the test_manager.
5. Call RUN_ALL_TESTS()
   This will invoke the google test execution engine to run all tests (or those specified with the --gtest_filter argument).
6. Shutdown the service_manager and the test_manager.



### Test Implementation
Google test fixtures are typically declared and defined in C++ source (.cpp) files. valapps tests are segregated by domain into one .cpp file per test fixture and compiled into
one library per domain. For example, all tests related to testing the Partial Reconfigure feature of AAL are defined in the file gtPartialReconfig.cpp. Furthermore,
the tests are compiled into one library, called libgtreconfigure.so.

If one or more of the tests in a test fixture require command line arguments, a test developer should use the SetUp method to get and parse those arguments. An example SetUp
method (used by the "Partial Reconfiguration" tests) is shown below. This example uses the test_context::args() method to get a reference to the raw arguments registered in the main function then use the arguments::operator()(const std::string &, char) to specify command line arguments used by the tests. 
It then calls arguments::parse() to have them parsed. This will allow for accessing them by their names used during argument specification.

        virtual void SetUp()
        {
            args()("bitstream1", 'a')
                  ("bitstream2", 'b').parse();
        }


Inside the test implementation, a test that is part of a fixture that derives from a test_context should use the get_service method to get a shared_ptr to a service_client object.
Once one or more service_client objects have been obtained, the test shall call functions part of that client API and use google test assertion (or expect) macros to assert a test
step has passed. The following example shows how parts of the Partial Reconfigure test may be implemented. It uses get_service method to get pointers to both the "Partial Reconfiguration"
service client as well as the "NLB0" service client. This example also uses google test macros to assert certain conditions have been met before continuing with the test logic. 


    reconfigure_client pr = get_service<reconfigure_client>("PR");
    ASSERT_TRUE(pr) << "Could not get PR interface";

    string bs1 = args().get_string("bitstream1");
    string bs2 = args().get_string("bitstream2");

    reconfigure_client::status_t status = pr->reconfigure(bs1, 1000, action::honor_owner);
    ASSERT_EQ(status, reconfigure_client::status_t::reconfigure_success) << "reconfigure not successful";

    // excercise the AFU via the nlb_client::loopback1 function
    nlb_client nlb = this->get_service<nlb_client>("NLB0");
    bool lb_success = nlb->loopback1(4, 2);
    ASSERT_TRUE(lb_success);
    nlb->release();
    service_client::status_t afu_status = nlb->status();    
    ASSERT_EQ(afu_status, service_client::status_t::released) << "AFU not released";



<a name="runit"/>
## Test Execution

### gtapp
The main function described above is used to build the test executable, called gtapp, which is used to execute tests
compiled in test libraries. The only required argument to gtapp is the name of the test library (specified with the -t|--testlib argument).
By default, gtapp looks for a services.json file in the current working directory for the service definitions to load 
before test execution but can be overridden with the -s|--services command line argument. 
Additional command line arguments that are processed first by gtapp are -h|--help and --gtest_help. The double-hyphen is used 
to tell gtapp to stop processing command line arguments and any arguments after that for either the google test
runtime or the valapp test fixture.
Typing gtapp --help will print out the general help for gtapp.

    >gtapp --help

    [--services|-s <value> ] --testlib|-t <value>  --gtest_help|-H --help|-h   -- [test arguments] | [gtest arguments]

### runit.py
The runit.py Python script is a wrapper for calling gtapp with one or more test libraries and uses a test specification file, in JSON
format, for listing out which test libraries to call along with any command line arguments necessary. By default, runit.py will
load tests from a file called [tests.json](tests.html) in the current working directory but can be overridden with the -f|--tests-file command 
line argument. Two more useful arguments for test execution are --filter and -d|--debug. The former can be used to pass google test
filters to the google test engine and will be applied to any and all test libraries loaded by gtapp. The latter is used to run 
gtapp in the gnu debugger (gdb) and can be useful when debugging tests and/or AAL runtime.

## Test Development and/or Execution Flows
The following scenarios are typical uses of the tools and executables part of the valapps codebase.

### Develop, Build and Run tests 
Using the [buildenv.py](#buildenv) described above, a developer can get his/her development environment ready by using it from the
valapps directory with the bootstrap command line argument.
    >./buildenv.py -B

Next, the developer can build the AALSDK from the same git clone and install the binaries into the .packages directory.
    >./buildenv.py -b -s ../

Once the two steps described above have been executed, the developer is ready to enter the development phase of valapps tests. 
He/she will use the -M|--make command line argument to build valapps. Once built, an env.sh file will be configured in the build
directory which can be used to get the execution environment ready.
    
    >source build/env.sh

This will setup the developer's environment so that he/she may call gtapp and any test libraries without specifying paths.
For example, say a developer is working on a valapps test called sw_mmio_09 built in the gtmmio library, 
the test can be run using the following command:

    >gtapp -t gtmmio -- --gtest_filter="*09"

In addition to using gtapp to run a test, one may choose instead to use runit.py to run a test from the tests.json file.
This is useful in running a given test without having to type out all command line arguments by hand.
For example, say a developer wants to run the gtreconfigure tests without having to specify the command line arguments
necessary for the tests, he/she would use the following command:

    >./runit.py -t gtreconfigure 


### Run Tests from pre-built binaries
Using [buildenv.py](#buildenv), valapps may be packaged into a tar.gz file and transferred to a target system for execution.
Once untarred on the target system, the runit.py script may be used to execute all tests in the tests.json file which will 
be found at the root of the target directory.

For example:

    >mkdir /tmp/valapps
    >cd /tmp/valapps
    >tar -zxvf /path/to/valapps.tar.gz
    >...
    >./runit.py 


The steps listed above would result in executing tests listed in the tests.json file (also found at the root of the target directory) 
and generate test results in gt*.xml files.

### Build and Run Tests as part of CI
A job in a CI build configuration can accomplish the building of valapps along with the test execution by using the following commands:

    >cd valapps
    >./buildenv.py -B -b -s ../ -clean -M
    >./runit.py

As mentioned above, a google test results file will be generated for each test library executed. The CI system may also be configured to 
look for and process these results files for processing and displaying as part of the build.



