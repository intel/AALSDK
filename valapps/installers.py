import tarfile
import tempfile
import subprocess
import os
import logging
import shlex

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

    p = subprocess.Popen(cmd, cwd=cwd, env=env, stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True)
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
        cwd = kwargs.pop('cwd', build_dir)
        args = [configure_script] + ["--{}={}".format(k,v) for k,v in kwargs.iteritems()]
        return run(' '.join(args), cwd=cwd)

    def make(self, directory, *args, **kwargs):
        targets = list(args) or ['all']
        if "makefile" in kwargs:
            targets = ["-m", kwargs["makefile"]] + targets
        logging.debug("making {} in directory: {}".format(' '.join(targets), directory))
        return run('/usr/bin/make {}'.format(' '.join(targets)), cwd=directory, env=kwargs.get("env"))

    def make_all(self, *args, **kwargs):
        root_dir = kwargs.pop("root", os.getcwd())
        has_makefile = lambda x : os.path.exists(os.path.join(root_dir, x, 'Makefile'))
        subdirs = filter(lambda x : os.path.isdir(x) and has_makefile(x), os.listdir(root_dir))
        return [self.make(os.path.join(root_dir, sub), *args, **kwargs) for sub in subdirs]


class base_installer(object):
    def install(filename, packages_dir):
        pass

class make_installer(base_installer):
    def install(self, rootdir, packages_dir, **kwargs):
        exit_code = 0
        m = maker()
        m.configure(rootdir, rootdir, prefix=packages_dir, cwd=rootdir)
        exit_code |= m.make(rootdir, cwd=rootdir)[0]
        exit_code |= m.make(rootdir, 'install', cwd=rootdir)[0]
        return exit_code


class cmake_installer(base_installer):
    def install(self, rootdir, packages_dir, **kwargs):
        exit_code = 0
        env = {"PATH": os.path.join(packages_dir, "bin"),
               "LD_LIBRARY_PATH": os.path.join(packages_dir, "lib")}
        env = kwargs.get("env", env)
        builddir = os.path.join(rootdir, "build")
        if not os.path.exists(builddir):
            os.mkdir(builddir)
        if not os.path.exists(os.path.join(builddir, "CMakeCache.txt")):
            exit_code |= run("cmake ../ -DCMAKE_INSTALL_PREFIX={} {}".format(packages_dir, kwargs.get("cmake_args", "")), cwd=builddir, env=env)[0]
        exit_code |= run("make", cwd=builddir, env=env)[0]
        exit_code |= run("make install", cwd=builddir, env=env)[0]
        return exit_code


INSTALLERS = { "make_installer" : make_installer,
               "cmake_installer" : cmake_installer }
def get(installer_name):
    return INSTALLERS.get(installer_name)()

