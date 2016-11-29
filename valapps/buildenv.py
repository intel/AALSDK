#!/usr/bin/env python
from os import path, getcwd, mkdir, environ, listdir, makedirs
from subprocess import check_output, CalledProcessError
import re
import json
import urllib2
import installers
import unpackers
import fetchers
import logging
import sys
import shutil

cache_dir = "~/.buildenv/cache"

def installed(package_info):
    try:
        if package_info.get('type') == 'executable':
            try:
                version = check_output('{} --version'.format(package_info['name']), shell=True)
            except CalledProcessError:
                return False
            version = re.findall('(\d+\.\d+(?:\.\d+)?)', version)
            if version:
                version = version[0].split('.')
                return version >= package_info.get('version', '9999.9.9').split('.')
        elif package_info.get('type') == 'library':
            libs = check_output('ldconfig -p', shell=True)
            if re.match('.*{}.*'.format(package_info['name']),libs, re.MULTILINE | re.IGNORECASE):
                return True
            for p in environ.get('LD_LIBRARY_PATH', '').split(':'):
                if p and path.exists(p) and filter(lambda f : package_info['name'] in f, listdir(p)):
                    return True
    except:
        pass
    return False

def buildenv(cwd):
    packages_dir = path.abspath(path.join(cwd, ".packages"))
    develop_dir = path.abspath(path.join(cwd, ".develop"))

    bindir = [path.join(dirname, "bin") for dirname in (packages_dir, develop_dir)]
    libdir = [path.join(dirname, "lib") for dirname in (packages_dir, develop_dir)]

    env = { "PATH": ":".join(bindir),
            "LD_LIBRARY_PATH": ":".join(libdir) }
    return env

def bootstrap(packages_file, package_list, install_dir):
    global cache_dir
    cwd = getcwd()
    packages_dir = install_dir

    cache_dir = path.expanduser(cache_dir)
    for dirname in [packages_dir, cache_dir]:
        if not path.exists(dirname):
            makedirs(dirname)
    with open(packages_file, 'r') as fd:
        packages = json.load(fd)

    if package_list:
        packages = filter(lambda p : p['name'] in package_list, packages)

    # get the packages that need to be fetched/installed
    packages = filter(lambda p : not installed(p), packages)
    exit_code = 0
    for package in packages:
        exit_code |= fetchers.get_package(package, packages_dir, cache_dir)

    return exit_code

class BuildError(Exception):
    pass

class SdkBuildError(BuildError):
    pass

def buildsdk(srcdir, install_dir):
    global cache_dir
    cache_dir = path.expanduser(cache_dir)
    exit_code = 0
    cwd = getcwd()
    packages_dir = install_dir
    cache_dir = cache_dir or path.join(cwd, ".cache")
    srcdir = path.abspath(srcdir or path.join(cache_dir, "aalsdk"))
    configure_exists = [path.exists(path.join(srcdir, x, 'configure')) for x in ['aaluser', 'aalkernel']]
    if not any(configure_exists):
        installers.run(path.join(srcdir, 'prep-build'), cwd=srcdir)

    aaluser_dir = path.join(srcdir, "aaluser")
    aalkernel_dir = path.join(srcdir, "aalkernel")
    aaluser_build = path.join(aaluser_dir, ".build")
    aalkernel_build = path.join(aalkernel_dir, ".build")
    m = installers.maker()
    logging.info("building aaluser")
    m.configure(aaluser_dir, aaluser_build, prefix=packages_dir)
    exit_code |= m.make(aaluser_build)[0]
    logging.info("installing aaluser")
    exit_code |= m.make(aaluser_build, "install")[0]
    logging.info("building aalkernel")
    m.configure(aalkernel_dir, aalkernel_build)
    exit_code |= m.make(aalkernel_build)[0]
    kernel_install = path.join(packages_dir, "kernel")
    if path.exists(kernel_install):
        shutil.rmtree(kernel_install)
    shutil.copytree(aalkernel_build, kernel_install)
    if exit_code != 0:
        raise SdkBuildError()

    return packages_dir

def buildit(aalsdk, **kwargs):
    exit_code = 0
    cwd = path.abspath(getcwd())
    packages_dir = path.join(cwd, ".packages")
    env = buildenv(getcwd())

    if kwargs.get('clean', False) == 'wipe':
        logging.info("wiping out build directory")
        shutil.rmtree("build")
    if not path.exists("build"):
        mkdir("build")
    configuration = kwargs.get('configuration', 'Release')
    cmake_cmd = "cmake ../ -DAALSDK={} -DCMAKE_INSTALL_PREFIX={} -DCMAKE_BUILD_TYPE={}".format(aalsdk, packages_dir, configuration)
    logging.info("configuring valapps build")
    exit_code |= installers.run(cmake_cmd, cwd="build", env=env)[0]

    if kwargs.get("clean", False) == "clean":
        logging.info("cleaning valapps build")
        exit_code |= installers.run("make clean", cwd="build")[0]
    elif kwargs.get("rebuild_cache", False):
        exit_code |= installer.run("make rebuild_cache", cwd="build")[0]
    logging.info("making valapps")
    exit_code |= installers.run("make", cwd="build")[0]
    logging.info("installing valapps")
    exit_code |= installers.run("make install", cwd="build")[0]
    if kwargs.get('package',False):
        cmd = 'cpack'
        if kwargs.get('generator'):
            cmd += ' -G ' + kwargs.get('generator')
        logging.info("packaging valapps")
        exit_code |= installers.run(cmd, cwd="build", env=env)[0]
    return exit_code

if __name__ == '__main__':
    cwd = getcwd()
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument('-p', '--packages', default='packages.json',
                        help='name of package file that lists packages to install')
    parser.add_argument('-P', '--package-list', nargs='*', default=[],
                        help='names of packages from package file to install')
    parser.add_argument('-B', '--bootstrap', action='store_true', default=False,
                        help='install packages from packages file')
    parser.add_argument('-k', '--aalsdk',
                        help='aalsdk installation root')
    parser.add_argument('-b', '--buildsdk', action='store_true', default=False,
                        help='builds the aalsdk and installs it')
    parser.add_argument('-s', '--aalsdk-src',
                        help='path to aalsdk src to build')
    parser.add_argument('-M', '--make', action='store_true', default=False,
                        help='make valapps')
    parser.add_argument('-c', '--clean', action='store_true', default=False,
                        help='clean valapps before building')
    parser.add_argument('--really-clean', action='store_true', default=False,
                        help='wipe out the build directory before the build')
    parser.add_argument('-C', '--configuration', default='Release', choices=['Debug', 'Release'],
                        help='Build configuration type. Default is Release')
    parser.add_argument('-K', '--make-package', action='store_true',
                        help='generate distribution binary package')
    parser.add_argument('-G', '--package-generator', choices=['7Z', 'DEB', 'RPM', 'STGZ', 'TBZ2', 'TGZ', 'TXZ', 'TZ', 'ZIP'],
                        help='cpack package generator to use')
    parser.add_argument('--loglevel', choices=['CRITICAL', 'ERROR', 'WARNING', 'INFO', 'DEBUG', 'NOTSET'], default='INFO',
                        help=argparse.SUPPRESS)
    parser.add_argument('--install-dir',
                        help=argparse.SUPPRESS)


    args = parser.parse_args()
    exit_code = 0
    logging.basicConfig(level=getattr(logging, args.loglevel))
    install_dir = path.join(cwd, ".packages")

    if args.install_dir:
        install_dir = args.install_dir

    if args.bootstrap:
        exit_code |= bootstrap(args.packages, args.package_list, install_dir)

    if args.buildsdk and args.aalsdk is None:
        sdk = buildsdk(args.aalsdk_src, install_dir)
    else:
        sdk = path.abspath(args.aalsdk or install_dir)

    if args.make:
        if args.really_clean:
            clean = 'wipe'
        elif args.clean:
            clean = 'clean'
        else:
            clean = False
        exit_code |= buildit(sdk, clean=clean,
                                  configuration=args.configuration,
                                  package=args.make_package,
                                  generator=args.package_generator,
                                  )
    sys.exit(exit_code)
