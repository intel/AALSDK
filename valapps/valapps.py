#!/usr/bin/env python
import argparse
import difflib
import glob
import xml.etree.cElementTree as ET
import importlib
import itertools
import json
import logging
import os
import re
import shlex
import socket
import subprocess
import sys
import unittest

import xml.etree.ElementTree as ET
from xml.dom import minidom

def run(cmd, **kwargs):
    cwd = kwargs.get("cwd")
    env = kwargs.get("env")
    if env is not None:
        env = dict(os.environ)
        for k,v in kwargs.get("env", {}).iteritems():
            if k in env:
                env[k] = ":".join([v, env[k]])
            else:
                env[k] = v

    if type(cmd) is not list:
        args = shlex.split(cmd)
    else:
        args = cmd
    p = subprocess.Popen(args, cwd=cwd, env=env, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    s,e = p.communicate()
    if p.returncode != 0:
        logging.warning("Command: {} returned non-zero exit code\n{}\n{}".format(cmd, s, e))
    else:
        logging.debug("Command: {} successful\n{}\n{}".format(cmd, s, e))
    return p.returncode, s, e

class maker(object):
    '''
    maker is a helper class for building the AALSDK with its current build system.
    maker has a prep method that will call 'prep_build'
    '''

    def prep(self, aalsdk_root):
        prep_script = os.path.join(aalsdk_root, "prep-build")
        assert(os.path.exists(prep_script))
        subprocess.call(prep_script, cwd=aalsdk_root)

    def configure(self, configure_dir, build_dir, **kwargs):
        if not os.path.exists(build_dir):
            os.makedirs(build_dir)
        elif os.path.exists(os.path.join(build_dir, 'Makefile')):
            logging.debug("Makefile already exists, skipping configuration step")
            return
        configure_script = os.path.abspath(os.path.join(configure_dir, "configure"))
        assert(os.path.exists(configure_script))
        args = [configure_script] + ["--{}={}".format(k,v) for k,v in kwargs.iteritems()]
        return run(args, cwd=build_dir)

    def make(self, directory, *args, **kwargs):
        targets = list(args) or ['all']
        if "makefile" in kwargs:
            targets = ["-m", kwargs["makefile"]] + targets
        logging.debug("making {} in directory: {}".format(' '.join(targets), directory))
        return run(['/usr/bin/make'] + targets, cwd=directory, env=kwargs.get("env"))

    def make_all(self, *args, **kwargs):
        root_dir = kwargs.pop("root", os.getcwd())
        has_makefile = lambda x : os.path.exists(os.path.join(root_dir, x, 'Makefile'))
        subdirs = filter(lambda x : os.path.isdir(x) and has_makefile(x), os.listdir(root_dir))
        return [self.make(os.path.join(root_dir, sub), *args, **kwargs) for sub in subdirs]

def disabled(fn):
    def wrapped(self):
        print "{} is disabled".format(fn.__name__)
        raise unittest.case.SkipTest("test is disabled")
    wrapped.__doc__ = fn.__doc__
    return wrapped

class params(object):
    '''
    Test decorator to allow parametric runs of a test by making the product of all sequences passed in.
    It is like making nested for loops on each sequence.
    '''
    def __init__(self, *args, **kwargs):
        self.args = list()
        for arg in args:
            if type(arg) is not list:
                self.args.append([arg])
            else:
                self.args.append(arg)
        self.kwargs = kwargs

    def __call__(self, fn):
        def inner(test, *args, **kwargs):
            for args in itertools.product(*self.args):
                fn(test, *args, **self.kwargs)
        inner.__doc__ = fn.__doc__
        return inner


class DetailResult(unittest.TextTestResult):
    '''
    The default TestResult class in unittest module does not capture successes. This class inherits from TestResult
    and overrides the addSuccess method to capture successes. It also has a method called writeToXml which will write
    the results in gtest xml format at the end of a suite being run.
    '''
    def __init__(self, stream, one, verbosity):
        unittest.TextTestResult.__init__(self, stream, one, verbosity)
        self.successes = list()
        self.alltests = list()

    def addError(self, test, err):
        unittest.TextTestResult.addError(self, test, err)
        self.alltests.append((test, "error", err))

    def addFailure(self, test, err):
        unittest.TextTestResult.addFailure(self, test, err)
        self.alltests.append((test, "failure", err))

    def addSuccess(self, test):
        self.successes.append(test)
        self.alltests.append((test, "success", None))

    def addSkip(self, test, reason):
        unittest.TextTestResult.addSkip(self, test, reason)
        self.alltests.append((test, "skipped", reason))

    def stopTestRun(self):
        unittest.TextTestResult.stopTestRun(self)
        self.writeToXml()

    def writeToXml(self):
        if self.testsRun > 0:
            filename = "{}_Results.xml".format(self._previousTestClass.__name__)
            root = ET.Element("testsuite", errors=str(len(self.errors)), failures=str(len(self.failures)), skips=str(len(self.skipped)), tests=str(self.testsRun))
            for test, status, err in self.alltests:
                node = ET.SubElement(root, "testcase", classname=test.__class__.__name__, name=test._testMethodName)
                if status != "success":
                    errnode = ET.SubElement(node, status, message="test {}".format(status))
                    errnode.text = str(err)
            tree = ET.ElementTree(root)
            from xml.dom import minidom
            xmlstring = minidom.parseString(ET.tostring(root)).toprettyxml(indent='  ')
            with open(filename, 'w') as fd:
                fd.write(xmlstring)

    def showAll(self):
        unittest.TextTestResult.showAll(self)

class test_suite(object):
    '''
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

        
    '''
    def diff(self, lhs, rhs):
        '''
        Performa unified diff between the lhs text and the rhs text. The result is the joining of the differing lines.

        '''
        return os.linesep.join(difflib.unified_diff(lhs.split(os.linesep), rhs.split(os.linesep)))
        
        
    def exe(self, exe, *args, **kwargs):
        '''
        Run an executable in a bash shell. The current working directory (cwd) will be the directory where this test class originated.
        If "exit" is passed in the kwargs, this will return the exit code, otherwise it will raise an exception on non-zero codes.
        If "stdout" is passed in the kwargs, this will return the stdout from the new process.
        The rest of *args and **kwargs will be passed in to Popen.
        '''
        if hasattr(self, "_testdir") and "cwd" not in kwargs:
            kwargs["cwd"] = self._testdir
        exe_args = ["/bin/bash", "-c", " ".join([exe] + kwargs.pop("args", []))]
        get_exitcode = kwargs.pop("exit", False)
        if kwargs.pop("stdout", False):
            return subprocess.check_output(exe_args, *args, **kwargs)
        else:
            print(exe)
            p = subprocess.Popen(exe_args, *args, stderr=subprocess.PIPE, **kwargs)
            if p.wait() != 0 and not get_exitcode:    
                error = "Command: {} exited with non-zero exit code ({}), stderr={}".format(exe, p.returncode, p.stderr.read())
                self.fail(error)
            return p.returncode 
            
            
    def stdout(self, exe, *args, **kwargs):
        '''
        Convenience method to call 'exe' and return captured stdout
        '''
        return self.exe(exe, *args, stdout=True, **kwargs)
    
    def exit_code(self, exe, *args, **kwargs):
        '''
        Convenience method to call exe and return the exit code
        '''
        return self.exe(exe, *args, exit=True, **kwargs)


    def _make_testcase(self, dirname):
        module = importlib.import_module(dirname)
        module_dict = module.__dict__
        test_methods = dict(filter(lambda kv : kv[0].startswith("test"), module_dict.items()))
        def newclass(name, **kwargs):
            return type(name, (test_suite, unittest.TestCase), kwargs)
        
        if 'setup' in module_dict:
            test_methods["setUp"] = module_dict["setup"]
        if 'teardown' in module_dict:
            test_methods["tearDown"] = module_dict["tearDown"]
        test_methods['_testdir'] = dirname
        name = "{}Tests".format(dirname)
        return newclass(name, __name__=name, **test_methods) 
    
    def discover_tests(self, root_dir):
        is_module = lambda x : os.path.exists(os.path.join(root_dir, x, '__init__.py'))
        subdirs = filter(lambda x : os.path.isdir(x) and is_module(x), os.listdir(root_dir))
        return subdirs

    def run_suite(self, **kwargs):
        root_dir = kwargs.pop("cwd", os.getcwd())
        test_cases = list()
        loader = unittest.TestLoader()
        subdirs = self.discover_tests(root_dir)

        def matches_patterns(name, patterns):
            if not patterns:
                return True
            for pattern in patterns:
                if re.match(pattern, name):
                    return True
            return False
        patterns = kwargs.get("runtests")

        for subdir in subdirs:
            if matches_patterns(subdir, patterns):
                test_cases.append(loader.loadTestsFromTestCase(self._make_testcase(subdir)))

        runner = unittest.TextTestRunner(verbosity=2, resultclass=DetailResult)
        results = list()      
        for test_case in test_cases:
            results.append(runner.run(test_case))

        return results

def consolidate_results(resultfile=None):
    results = []
    alltests = ET.Element("testsuite")
    counts = dict(errors=0, failures=0, skips=0, tests=0)
    for result in glob.glob("*.xml"):
        tree = ET.parse(result)
        root = tree.getroot()
        for a in ("errors", "failures", "skips", "tests"):
            counts[a] += int(root.attrib.get(a, 0))

    for testcase in root:
            result = {"classname":testcase.attrib["classname"],
                      "name":testcase.attrib["name"]}
            alltests.append(testcase)
            if len(testcase):
                try:
                    sub = testcase[0]
                except IndexError:
                    sub = None
                result["status"] = sub.tag
                result["message"] = sub.text
            else:
                result["status"] = "pass"
            results.append(result)
    for k,v in counts.iteritems():
        alltests.attrib[k] = str(v)
    alltests.attrib["name"] = socket.gethostname()
    xmlstring = minidom.parseString(ET.tostring(alltests)).toprettyxml(indent='    ')
    if resultfile is None:
        return xmlstring
    else:
        with open(resultfile, "w") as fd:
            fd.write(xmlstring)

                
def driver_loaded():
    output = subprocess.check_output("lsmod") 
    if re.search("aalbus.*ccipciedrv,aalrmc,aalrms", output):
        return True
    return False


def build_sdk(srcdir):
    configure_exists = [os.path.exists(os.path.join(srcdir, x, 'configure')) for x in ['aaluser', 'aalkernel']]
    if not any(configure_exists):
        subprocess.call([os.path.join(srcdir, 'prep-build')], cwd=srcdir)
    
    aaluser_dir = os.path.join(srcdir, "aaluser")
    aalkernel_dir = os.path.join(srcdir, "aalkernel")
    prefix=os.path.join(os.getcwd(), ".build", "aaluser.install")
    m = maker()
    m.configure(aaluser_dir, ".build/aaluser", prefix=prefix) 
    m.make(".build/aaluser")
    m.make(".build/aaluser", "install")
    m.configure(aalkernel_dir, ".build/aalkernel")
    m.make(".build/aalkernel")
    return prefix

def unload_driver(builddir, **kwargs):
    if driver_loaded():
        subprocess.call("sudo {}".format(os.path.join(builddir, "rmdrv")), shell=True) 

def load_driver(builddir, **kwargs):
    unload_driver(builddir, **kwargs)
    subprocess.call("sudo {} {}".format(os.path.join(builddir, "insdrv"), kwargs.get("driver")), shell=True) 

def makeit(*args, **kwargs):
    m = maker()
    
    prefix = kwargs.get("aalsdk")
    
    if prefix is None:
        raise RuntimeError("Don't know what prefix to use to link valapps with. Either build the source in this repository or specify the sdk installation path with --aalsdk")

    env = {'prefix':prefix}

    codes = []
    codes.append(m.make("valapp", env=env)[0])
    codes.append(m.make("appbase", env=env)[0])
    codes.append(m.make("vallib", env=env)[0])
    codes.append(m.make("AFU_Reset", env=env)[0])
    codes.append(m.make("DMA_Buffer", env=env)[0])
    codes.append(m.make("Error", env=env)[0])
    codes.append(m.make("MMIO_Mapping", env=env)[0])
    codes.append(m.make("PartialReconf_Power", env=env)[0])
    codes.append(m.make("Partial_Reconfig/PR_SingleApp", env=env)[0])
    codes.append(m.make("Partial_Reconfig/PR_TwoApp/pr_app", env=env)[0])
    codes.append(m.make("Partial_Reconfig/PR_TwoApp/nlb_app", env=env)[0])
    codes.append(m.make("Power_Thermal", env=env)[0])
    codes.append(m.make("RAS", env=env)[0])
    codes.append(m.make("Read_Perf_Counters", env=env)[0])
    codes.append(m.make("Signal_Tap", env=env)[0])
    codes.append(m.make("UMsg_Allocation", env=env)[0])
    assert all(codes) == 0, "One or more dependencies failed to build"

def cleanit():
    m = maker()
    return m.make_all("clean")

def testit(*args, **kwargs):
    sys.path.append(os.getcwd())
    suite = test_suite()
    return suite.run_suite(**kwargs)

def list_tests():
    suite = test_suite()
    subdirs = suite.discover_tests(os.getcwd())
    for subdir in subdirs:
        module = importlib.import_module(subdir)
        module_dict = module.__dict__
        test_methods = dict(filter(lambda kv : kv[0].startswith("test"), module_dict.items()))
        print "Test Directory: {}".format(subdir)
        for k,v in test_methods.iteritems():
            print ' '*3, k
            print ' '*3, '_'*len(k)
            if v.__doc__ is not None:
                print v.__doc__
            else:
                print ' '*3, 'No documentation found'
    
            print ' '*3, '_'*len(k)
            print
 
def cache_rtl_paths(rtldir):
    mode0 = glob.glob(os.path.join(rtldir, "*0.rbf"))
    mode3 = glob.glob(os.path.join(rtldir, "*3.rbf"))
    rtl = {"rtldir":rtldir}
    if mode0:
        rtl["mode0"] = mode0
        rtl["mode0idx"] = 0
    if mode3:
        rtl["mode3"] = mode3
        rtl["mode3idx"] = 0

    with open("rtl.json", "w") as fd:
        json.dump(rtl, fd, indent=4, sort_keys=True)



if __name__ == "__main__":
    logformat = "%(asctime)-15s %(levelname)s %(message)s"
    logging.basicConfig(level=logging.INFO, format=logformat)
    parser = argparse.ArgumentParser()
    parser.add_argument("-k", "--aalsdk", 
                        help='''
                                Location of aalsdk installation to link with.
                                If not specified, this script will build both aaluser/aalkernel from this codebase
                                (Assuming that this directory is parallel to the aforementioned directories).
                             ''')
    parser.add_argument("-t", "--test-only", default=False, action="store_true", 
                        help="Run tests only, don't try to rebuild valapps")
    parser.add_argument("-m", "--make-tests", default=[], nargs="*",
                        help="Make only the tests in the list")
    parser.add_argument("-M", "--make-only", default=False, action='store_true',
                        help="Make only the tests. Do not run tests")
    parser.add_argument("-b", "--build-sdk", action="store_true", default=False,
                        help="Build sdk either in this code base or that pointed to by --sdksrc")
    parser.add_argument("-S", "--sdksrc",
                        help="root directory containing aalsdk source code")
    parser.add_argument("-c", "--clean-only", default=False, action='store_true',
                        help="Run 'make clean' on the tests. Do not run tests")
    parser.add_argument("-r", "--run-tests", default=[], nargs="*", 
                        help="Run tests matching pattern(s)")
    parser.add_argument("-L", "--load-driver", action="store_true", default=False, 
                        help="Load kernel driver from kernel build")
    parser.add_argument("-U", "--unload-driver", action="store_true", default=False,
                        help="Unload kernel driver loaded from kernel build")
    parser.add_argument("-d", "--driver", choices=["cci", "cci-sim"], default="cci", 
                        help="Which driver to load (if loading driver after build). Default is cci-sim")
    parser.add_argument("-l", "--list", action="store_true", default=False,
                        help="Lists tests discovered and the documentation (if found) for each test.")
    parser.add_argument("-R", "--rtl",
                        help='''
                                 Path to directory containing RTL files (for NLB *.rbf files).
                                 Look for *0.rbf and *3.rbf files in this directory and cache them in rtl.py.
                                 NOTE: If more than one file exists for each wildcard (*0.rbf and *3.rbf) in the
                                 given path, this script will cache the path to the first one found.
                             ''')
    parser.add_argument("--consolidate-results",
                        help="Consolidate all results to one file (specified by this argument)")
    parser.add_argument("--help-full", action='store_true', default=False,
                        help="shows detailed help information gernated from pydoc strings in this file")
    
    
    
    args = parser.parse_args() 
    thisdir = os.path.abspath(os.path.dirname(__file__))
    if args.help_full:
        parser.print_help()
        import pydoc
        t = pydoc.TextDoc()
        print(t.document(sys.modules[__name__]))
    elif args.list:
        list_tests()
    elif args.clean_only:
        cleanit()
    elif args.rtl:
        cache_rtl_paths(args.rtl)
    elif args.unload_driver:
        if os.path.exists(".build/aalkernel/rmdrv"):
            unload_driver(".build/aalkernel", driver=args.driver)
        else:
            logging.error("Asked to unload driver but don't know which one to load")
    elif args.load_driver:
        if os.path.exists(".build/aalkernel/insdrv"):
            load_driver(".build/aalkernel", driver=args.driver)
        else:
            logging.error("Asked to load driver but don't know which one to load")
    else:
        if args.aalsdk:
            prefix = args.aalsdk
        elif args.build_sdk:
            sdksrc = args.sdksrc or os.path.abspath(os.path.dirname(os.path.abspath(os.path.dirname(__file__))))
            prefix = build_sdk(sdksrc)
            sys.exit(0)
        elif os.path.exists(".build/aaluser.install"):
            prefix = os.path.abspath(".build/aaluser.install")
        else:
            prefix = None


        if not args.test_only or args.make_only:
            makeit(aalsdk=prefix)
        if not args.make_only:
            os.environ['LD_LIBRARY_PATH'] =  ':'.join([os.environ.get('LD_LIBRARY_PATH', ''), os.path.join(thisdir, 'valapp')])
            testlist = args.run_tests
            if os.path.exists("tests.json") and not testlist:
                try:
                    with open("tests.json", "r") as fd:
                        testdata = json.load(fd)
                    testlist = testdata["tests"]
                except:
                    testlist = []
                
            testit(aalsdk=args.aalsdk, runtests=testlist)
        if args.consolidate_results:
            consolidate_results(args.consolidate_results)
