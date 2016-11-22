#!/usr/bin/env python
import argparse
import json
import os
import subprocess
import sys
import fetchers
import logging
from datetime import datetime

def fetch_collateral(collateral_list):
    for collateral in collateral_list:
        logging.info("fetching collateral: %s", collateral["name"])
        fetchers.get_package(collateral)



def get_env(rootdir):
    bindir = os.path.join(rootdir, "bin")
    libdir = os.path.join(rootdir, "lib")
    tstdir = os.path.join(rootdir, "tests")
    return { "PATH":bindir,
             "LD_LIBRARY_PATH" : ":".join([libdir, tstdir])}


if __name__ == '__main__':
    cwd = os.path.abspath(os.getcwd())
    rootdir = os.path.abspath(os.path.dirname(__file__))
    if os.path.exists(os.path.join(rootdir, ".packages")):
        rootdir = os.path.join(rootdir, ".packages")
    parser = argparse.ArgumentParser()

    parser.add_argument("-d", "--debug", action="store_true", default=False,
                        help="run in debugger")
    parser.add_argument("-f", "--tests-file", default="tests.json",
                        help="run tests listed in json file")

    parser.add_argument("-t", "--tests", nargs="*", default=[],
                        help="run these tests only")

    parser.add_argument("-l", "--list-file", action="store_true", default=False,
                        help="list tests specified in test file")

    parser.add_argument("-L", "--list-libraries", action="store_true", default=False,
                        help="list tests specified in test directory")

    parser.add_argument("-C", "--configure-rtl", action="store_true", default=False,
                        help="configures tests file with paths to rtl files")

    parser.add_argument("-r", "--rtl-dir", default=os.path.join(rootdir, "rtl"),
                        help="directory containing rtl files to use")
    parser.add_argument("--filter", 
                        help="adds filter to tests being run")

    args = parser.parse_args()
    env = get_env(rootdir)

    with open(args.tests_file, 'r') as fd:
        scenario = json.load(fd)
        tests = scenario['tests']

    if args.configure_rtl:
        for test in tests:
            testargs = test.get("args", {})
            for k,v in testargs.iteritems():
                if k.startswith("bitstream") or k.startswith("rtl"):
                    testargs[k] = os.path.join(args.rtl_dir, os.path.basename(v))
        with open(args.tests_file, "w") as fd:
            json.dump(tests, fd, indent=4)
        sys.exit(0)

    if args.list_file:
        print('\n'.join([t["name"] for t in tests]))
        sys.exit(0)

    if args.list_libraries:
        print('\n'.join(os.listdir(os.path.join(rootdir, "tests"))))
        sys.exit(0)

    if args.tests:
        tests = filter(lambda t : t["name"] in args.tests, tests)
    
    if 'collateral' in scenario:
        fetch_collateral(scenario['collateral'])

    for test in tests:
        if test.get('disabled', False):
            print("skipping test: {}".format(test["name"]))
        else:
            ts = "{:%Y-%m-%d_%H_%M_%S.%f}".format(datetime.now())
            cmd = "gtapp -t {} -- --gtest_output=xml:{}_{}.xml ".format(test["name"], test["name"], ts)
            if "args" in test:
                cmd += " ".join(["--{}={}".format(k,v) for k,v in test["args"].iteritems()])
            test_filter = test.get("filter")
            if test_filter or args.filter:
                cmd += ' --gtest_filter="{}"'.format(":".join(filter(lambda f : f is not None, [test_filter, args.filter])))
            if args.debug:
                cmd = "/usr/bin/gdb --args " + cmd
            subprocess.call(cmd, env=env, shell=True)


