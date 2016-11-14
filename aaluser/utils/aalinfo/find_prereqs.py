#!/usr/bin/python
#
#  Copyright(c) 2014-2016, Intel Corporation
#
#  Redistribution  and  use  in source  and  binary  forms,  with  or  without
#  modification, are permitted provided that the following conditions are met:
#
#  * Redistributions of  source code  must retain the  above copyright notice,
#    this list of conditions and the following disclaimer.
#  * Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#  * Neither the name  of Intel Corporation  nor the names of its contributors
#    may be used to  endorse or promote  products derived  from this  software
#    without specific prior written permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
#  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,  BUT NOT LIMITED TO,  THE
#  IMPLIED WARRANTIES OF  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
#  ARE DISCLAIMED.  IN NO EVENT  SHALL THE COPYRIGHT OWNER  OR CONTRIBUTORS BE
#  LIABLE  FOR  ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR
#  CONSEQUENTIAL  DAMAGES  (INCLUDING,  BUT  NOT LIMITED  TO,  PROCUREMENT  OF
#  SUBSTITUTE GOODS OR SERVICES;  LOSS OF USE,  DATA, OR PROFITS;  OR BUSINESS
#  INTERRUPTION)  HOWEVER CAUSED  AND ON ANY THEORY  OF LIABILITY,  WHETHER IN
#  CONTRACT,  STRICT LIABILITY,  OR TORT  (INCLUDING NEGLIGENCE  OR OTHERWISE)
#  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,  EVEN IF ADVISED OF THE
#  POSSIBILITY OF SUCH DAMAGE.
# ****************************************************************************


#############################################################################
#                                                                           #
#                                  Imports                                  #
#                                                                           #
#############################################################################

from __future__ import print_function
try:
   from future_builtins import filter
except ImportError:
   pass
from functools import reduce
import re, sys, os, copy, types
import subprocess as proc
from collections import namedtuple, defaultdict, OrderedDict
import getopt
from itertools import chain


#############################################################################
#                                                                           #
#                               Custom Types                                #
#                                                                           #
#############################################################################

unameT      = namedtuple("unameT",      ["sysname", "nodename", "release",
                                          "version", "machine"])
filterT     = namedtuple("filterT",     ["pattern", "list"])
pkg_fieldsT = namedtuple("pkg_fieldsT", ["name", "version", "depends"])
data_itemT  = namedtuple("data_itemT",  ["version", "depends"])
conflictT   = namedtuple("conflictT",   ["candidates", "message"])


#############################################################################
#                                                                           #
#                             Custom Decorators                             #
#                                                                           #
#############################################################################

def Singleton( cls ):
   """ Turn any class into a singleton, allowing for class properties. """
   instances = {}
   def getinstance(*args, **kwargs):
      if cls not in instances:
         if 0 >= len(args) and 0 >= len(kwargs):
            # Allow access to "class statics" before full instanciation.
            # All classes that use this decorator should do a "sort-cut"
            # initialization if the constructor is given no arguments.
            # This is especially useful when coping propterties (see below).
            return cls()
         # Do a "full" instanciation and save the one and only "real" object.
         instances[cls] = cls(*args, **kwargs)
      return instances[cls]
   return getinstance

#----------------------------------------------------------------------------
def StaticVar( **kwargs ):
   """ Attach "static" variables to a function or class. """
   def InitializeStatics( func ):
      for k in kwargs:
         setattr( func, k, kwargs[k] )
      return func
   return InitializeStatics

#----------------------------------------------------------------------------
@StaticVar( pycode_tmplate = """
def get_it( x ):
   return getattr( anchor, "%(base)s")["%(key)s"]
def set_it( x, v ):
   getattr( anchor, "%(base)s" )["%(key)s"] = v
""" )
def hide_dict_prop( anchor, hidden_dict, key ):
   globNS = {'anchor': anchor}
   pycode = hide_dict_prop.pycode_tmplate % {'base':hidden_dict, 'key' : key}
   exec( pycode, globNS )
   setattr( anchor, key, property(globNS["get_it"], globNS["set_it"]) )

#----------------------------------------------------------------------------
def StaticVarProps( **kwargs ):
   """ like "StaticVar" but also will turn the keys of any dict used as
       an argument value into properites that access that dict attached
       as a static variable to a function or class. """
   def InitializeStatics( func ):
      for svar, svar_val in kwargs.items():
         setattr( func, svar, kwargs[svar] )
         if "dict" == type(svar_val).__name__:
            for k, v in svar_val.items():
               hide_dict_prop( func, svar, k )
      return func
   return InitializeStatics

#----------------------------------------------------------------------------
def CopyProps( **kwargs  ):
   """ Copy properties from one class to another. """
   def copy_properties( func ):
      for from_class_name, props_to_copy in kwargs.items():
         from_class_obj = sys.modules[__name__].__dict__[from_class_name]
         if 'function' == type(from_class_obj).__name__:
            # If the from_class_obj is a function, then the @CopyProps
            # decorator was used "in front of" a @Singleton decorator.
            # In this case, the function must be involked in order to obtain
            # an object through which the actual class object is accessed.
            from_class_obj = from_class_obj().__class__
         for prop in props_to_copy:
            a = getattr( from_class_obj, prop );
            setattr( func, prop, a )
      return func
   return copy_properties


#############################################################################
#                                                                           #
#                                  Classes                                  #
#                                                                           #
#############################################################################

#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#
# CLI -- Command Line Interface
#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#

@Singleton
@StaticVarProps( options = { "verbose" : 1,      "details" : False,
                             "progress" : False, "color" : False,
                             "prereqs" : None,   "flat": False,
                             "levels": False,   "simple": False,
                           } )
class CLI (object):
   """ Deal with the command line interface. """

   _usage_msgs = [ """
Usage: %(nm)s [-h | --help] [-v | --verbose] [-q | --quiet]
       %(sp)s [-d | --details] [-. | --progress] [-s | --simple]
       %(sp)s [-c <color> | --color=<color>]
       %(sp)s [-p <fname> | --prereqs=<fname>] %(sp)s
       %(sp)s [[-f | --flat] | [-l | --levels]]"""[1:], """
-h, --help ..... Show this usage message and exit.
-v, --verbose .. Increase the verbosity level by 1.  May be repeated.
-q, --quiet .... Decrease the verbosity level by 1.  May be repeated.
-d, --details .. Show which packages depend on a given package.
-., --progress . Show progress by printing dots to /dev/tty.
-c, --color .... Use <color> ansi color code ("31;47" if given as '') to
                 highlight any "Not Installed" message.
-p, --prereqs .. Get prerequisites from file <fname>, stdin if '-'.
-f, --flat ..... Show the versions of the prerequisites only, do not recurse.
-l, --levels ... Show the recursion level for each package.
-s, --simple ... Simplify the output. (conflicts with -d and -l)"""[1:]]

   _usage_conflicts = [
      conflictT("vs", '"verbose" and "simple" can\'t both be used.'),
      conflictT("fl", '"flat" and "levels" can\'t both be used.'),
      conflictT("ds", '"details" and "simple" can\'t both be used'), ]

   #-------------------------------------------------------------------------
   def help_and_exit( self, rc ):
      print( self._usage_msgs[0] % self._help_args );
      if 0 == rc:
         print( self._usage_msgs[1] );
      sys.exit( rc );

   #-------------------------------------------------------------------------
   def __init__( self, *args, **kwargs ):
      (self._exe_from, self._exe_name) = os.path.split( sys.argv[0] )
      self._cmd_line = sys.argv[1:]
      self._help_args = {'nm': self._exe_name, 'sp': " " * len(self._exe_name)}
      if 0 >= len(args) and 0 >= len(kwargs):
         return

      try:
         opts, args = getopt.getopt(self._cmd_line, "hvqd.c:p:fls", [
                   x.split()[0] for x in ["          |||||| | ||| ",
                                          "help -----+||||| | ||| ",
                                          "verbose ---+|||| | ||| ",
                                          "quiet ------+||| | ||| ",
                                          "details -----+|| | ||| ",
                                          "progress -----+| | ||| ",
                                          "color= --------+ | ||| ",
                                          "prereqs= --------+ ||| ",
                                          "flat ------------- +|| ",
                                          "levels -------------+| ",
                                          "simple --------------+ "][1:]] )

      except getopt.GetoptError as err:
         Console().error(err, help=1)

      flags = set()
      for o, a in opts:
         if o in ("-h", "--help"):
            self.help_and_exit( 0 )
         elif o in ("-v", "--verbose"):
            flags.add('v')
            self.verbose += 1
         elif o in ("-q", "--quiet"):
            flags.add('q')
            self.verbose -= 1
         elif o in ("-d", "--details"):
            flags.add('d')
            self.details = True
         elif o in ("-.", "--progess"):
            flags.add('.')
            self.progress = True
         elif o in ("-c", "--color"):
            flags.add('c')
            if 0 > len(a) and '-' == a[0]:
               msg = 'option "%s" must have an argument' % (o)
               Console().error( msg, help=2 )
            self.color = a
            Console().set_highlight()
         elif o in ("-p", "--prereqs"):
            flags.add('p')
            if '-' == a[0] and len(a) != 1:
               msg = 'option "%s" must have an argument' % (o)
               Console().error( msg, help=3 )
            if "-" == a:
               source = 'stdin'
            else:
               source = 'file "%s"' % (a)
            guess = OS_Release().guess()
            if None == guess:
               msg = ("Asked to read prerequisites from %s, but" +
                     "can't tell which package manager to use.") % (source)
               Console().error( msg, rc=4 )
            try:
               if "-" == a:
                  raw_prereqs = sys.stdin.read()
               else:
                  f = open(a)
                  raw_prereqs = f.read()
                  f.close()
            except:
               msg = 'Failed to read prerequisites from "%s"' % (source)
               Console().error( msg, rc=5 )
            cooked_prereqs = list(map(lambda x: re.sub(r'^\s*([^\s]*)\s/*$',
                                                       '\1', x),
                                      raw_prereqs.rstrip().split('\n')))
            self.prereqs = cooked_prereqs
            Console().print("Using prerequisites read from", source, lvl=1)
         elif o in ("-f", "--flat"):
            flags.add('f')
            self.flat = True
         elif o in ("-l", "--levels"):
            flags.add('l')
            self.levels = True
         elif o in ("-s", "--simple"):
            flags.add('s')
            self.simple = True
            self.verbose = 0
         else:
            Console().error('Unknown option: "%s"' % (o), help=6)

      if 0 < len(args):
         msg = "unknown trailing command line argument(s): "
         msg += " ".join(args)
         raise getopt.GetoptError(msg)

      for conflict in self._usage_conflicts:
         conflicted = True
         for f in conflict.candidates:
            if f not in flags:
               conflicted = False
               break
         if conflicted:
            Console().error(conflict.message, help=7)

#End class CLI

#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#
#  Console -- Console output
#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#

@Singleton
@CopyProps( CLI = ["verbose", "details", "progress", "color", "flat", "levels",
                 "simple"] )
class Console (object):
   """ Encapsulate access to console output. """

   vlevels = { 'always': 0, 'warnings': 1, 'dump_db': 3 }
   my_keys = [ 'help', 'rc', 'lvl' ]

   #-------------------------------------------------------------------------
   def __init__( self, *args, **kwargs ):
      if 0 >= len(args) and 0 >= len(kwargs):
         return
      self._width = args[0]
      self._dot_count = 0
      self._tty = open('/dev/tty', "wb", 0)
      self._not_installed = "!!! Not Installed !!!"
      self.set_highlight()

   #-------------------------------------------------------------------------
   def set_highlight( self ):
      if False != self.color:
         ansi_highlight = self.color if self.color != '' else "31;47"
         self._not_installed = "\033[{}m{}\033[0m".format(ansi_highlight,
                                                          self._not_installed)

   #-------------------------------------------------------------------------
   @property
   def max_name_len( self ):
      return self._max_name_len

   #-------------------------------------------------------------------------
   @max_name_len.setter
   def max_name_len( self, value ):
      self._max_name_len = value + 1

   #-------------------------------------------------------------------------
   def tick( self ):
      # When this script was new, it was much s-l-o-w-e-r and ticks where nice.
      if not self.progress:
         return
      self._tty.write(b'.');
      self._tty.flush()
      self._dot_count += 1
      if self._dot_count >= self._width:
         self.flush()

   #-------------------------------------------------------------------------
   def warning( self, *args, **kwargs ):
      arglist = list(args)
      arglist[0] = "WARNING: " + arglist[0]
      kwargs2 = dict(kwargs)
      kwargs2['lvl'] = self.vlevels['warnings']
      self.print( *arglist, **kwargs2 );

   #-------------------------------------------------------------------------
   def error( self, *args, **kwargs ):
      self.print( "ERROR:", *args, **kwargs );
      if 'help' in kwargs:
         CLI().help_and_exit( kwargs['help'] );
      if 'rc' in kwargs:
         sys.exit( kwargs['rc'] )

   #-------------------------------------------------------------------------
   def row_out( self, pkg, xlist ):

      def level_out( fmt ):
         if self.levels:
            lvl = xlist.package_levels[pkg]
            if None == lvl:
               lvl_out = " " * len(fmt % (0))
            else:
               lvl_out = fmt % (lvl)
            print( lvl_out, end="" )

      row_data = PackageDB()[pkg]
      if '' == row_data.version and None == row_data.depends:
         version = self._not_installed
      else:
         version = row_data.version
      if self.simple:
         if self.levels:
            level_out( "%02d, " )
         print( "%s, %s" % (pkg, version) )
      else:
         if self.levels:
            level_out( "%02d " )
         print( pkg, '.'*(self._max_name_len - len(pkg)), version )
      if self.details:
         x = xlist.get_pkgs_that_require(pkg).keys()
         x.sort()
         for fpkg in x:
            if fpkg in PackageDB() and '' != PackageDB()[fpkg].version:
               print( '\t', fpkg, " == ", PackageDB()[fpkg].version )
            else:
               print( '\t', fpkg )

   #-------------------------------------------------------------------------
   def _print_list_wrapped( self, l, indent=12 ):
      start_line = "\n" + " " * indent
      if 0 >= len(l):
         print( "]" );
         return
      mybuffer = start_line
      for item in sorted( l ):
         item_out = item.__repr__()
         if (len(mybuffer) + len(item_out) + 2) >= self._width \
         and mybuffer != start_line:
            print( mybuffer, end="" )
            mybuffer = start_line
         mybuffer += item_out + ', '
      if indent < len(mybuffer):
            print( mybuffer, end="" )
      print( ' ]' )

   #-------------------------------------------------------------------------
   def print_dbentry( self, k, v ):
      print('name: "%s"  version: "%s"  depends:  [' % (k, v.version), end='')
      self._print_list_wrapped( v.depends )

   #-------------------------------------------------------------------------
   def print_cap_pkglist( self, k, v ):
      print('capability: "%s"  provided by: [' % (k), end='')
      self._print_list_wrapped( v )

   #-------------------------------------------------------------------------
   def print( self, *args, **kwargs ):
      if "lvl" in kwargs and kwargs["lvl"] > self.verbose:
         return
      if self._dot_count > 0:
         self.flush()
      print( *args, **{k: kwargs[k] for k in kwargs if k not in self.my_keys} )

   #-------------------------------------------------------------------------
   def flush( self ):
      if 0 < self._dot_count:
         self._tty.write(b'\n')
         self._dot_count = 0

#End class Console

#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#
# StandardPrereqs -- Prerequisites from the aal documentation.
#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#

class StandardPrereqs ( object ):

   # Standard AAL SDK Prerequisites...   from:
   # "BDW + FPGA Beta Release 5.0.3 Software Installation Guide"
   # AKA AAL Beta Installation Guild"

   # Centos 6, 7
   _Centos   = [ "make", "gcc", "gcc-c++", "libstdc++", "flex", "ncurses",
                 "ncuress-devel", "autoconf", "libtool", "libtoo-ltdl-devel",
                 "m4", "kernel-devel", "numpy"]

   # Debian 6, 7
   _Debian   = [ "build-essential", "flex", "libncurses5-dev", "automake",
                 "libltdl-dev", "linux-headers-amd64", "python-numpy" ]

   # Fedora 22, RHEL 7, 8
   _Fedora   = [ "make", "gcc", "gcc-c++", "kernel", "libstdc++", "flex",
                 "ncurses", "ncurses-devel", "autoconf", "automake", "libtool",
                 "libtool-ltdl-devel", "m4", "kernel-devel" ]

   # openSUSE 13.1, LEAP-42.1
   _openSuSE = [ "make", "gcc", "gcc-c++", "libstdc++6", "flex", "autoconf",
                 "ncurses-devel", "automake", "libtool", "m4", "kernel-devel" ]

   # Ubuntu 14.04 LTS
   _Ubuntu   = [ "build-essential", "flex", "libncurses5-dev", "automake",
                 "libltdl-dev", "linux-headers-generic" ]

   # Search this table, based on info from /etc/os-release, to pick one
   # of the above lists of prerequisites.  This may seem a bit complex,
   # but the idea is to make it extreamly simple to associate a given
   # combination of os name and version with a list of prerequisites.

   _distro_matrix = {
   #  ID            VERSION FILTER        PREREQUISITES
     "centos"   : [ filterT(r'6(\..*)?' , _Centos),
                    filterT(r'7(\..*)?' , _Centos),
                    filterT(r'.*'       , _Centos),   ],

     "debian"   : [ filterT(r'6(\..*)?' , _Debian),
                    filterT(r'7(\..*)?' , _Debian),
                    filterT(r'.*'       , _Debian),   ],

     "rhel"     : [ filterT(r'7(\..*)?' , _Fedora),
                    filterT(r'8(\..*)?' , _Fedora),
                    filterT(r'.*'       , _Fedora),   ],

     "fedora"   : [ filterT(r'22(\..*)?', _Fedora),
                    filterT(r'.*'       , _Fedora),   ],

     "opensuse" : [ filterT(r'13\.1'    , _openSuSE),
                    filterT(r'.*'       , _openSuSE), ],

     "leap"     : [ filterT(r'42\.1'    , _openSuSE),
                    filterT(r'.*'       , _openSuSE), ],

     "ubuntu"   : [ filterT(r'14\.04'   , _Ubuntu),
                    filterT(r'.*'       , _Ubuntu),   ],
   }

   def __init__( self, os_release ):
      try:
         os_id = os_release['ID']
         possible_prereqs_for_os = self._distro_matrix[os_id]
         ver = os_release['VERSION_ID']
         for tryver in possible_prereqs_for_os:
            if re.match(tryver.pattern, ver):
               Console().print("Using prerequisites from",
                                    os_release['PRETTY_NAME'], lvl=1)
               self.prereqs = tryver.list
               return
      except:
         pass
      msg = 'I couldn\'t figure out what prerequisites to use.'
      Console().error( msg, 8 )

#End class StandardPrereqs

#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#
# PackageDB  --  Package Database (with capability cache if RPM)
#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#

@Singleton
class PackageDB ( object ):
   """ Collect and distribute information about all (installed?) packages. """

   _rpm_delimiter = "@-@"
   _rpm_cmd =  [ 'rpm', '-qa', '--qf', '%{NEVRA}\t%{VERSION}\t[%{REQUIRES},]'
                                       + _rpm_delimiter + ',[%{PROVIDES},]\n' ]
   _dpkg_cmd = [ 'dpkg-query', '--show', '-f',
                                      '${Package}\t${Version}\t${Depends}\n']
   _os_package_managers = {
     "centos" : _rpm_cmd,  "debian"   : _dpkg_cmd, "rhel"   : _rpm_cmd,
     "fedora" : _rpm_cmd,  "opensuse" : _rpm_cmd,  "leap"   : _rpm_cmd,
     "ubuntu" : _dpkg_cmd, }
   _sixteenK = 16 * 1024

   # NOTE: Some package managers, like "rpm", demand an extra step when mapping
   # a package to a list of packages it depends on.  The extra step is needed
   # because these package managers do not return a list of package names when
   # asked for a package's "depends", but rathar a list of cookies that need
   # futher processing to get package names.  PackageDB's "transmogrify" method
   # is used to perform this extra step.  If a sane package manager like "dpkg"
   # is used, then this method will be set to a simple "pass through" function.
   #    In the case of "rpm", the cookies are called "capabilities" and the
   # "transmogrify" method is set to the "_capability2packages" method, which
   # will ask "rpm" to return a list of packages that supply any of the given
   # capabilities.  This was vary slow: 30 seconds in tests.  So a cache was
   # added: this cut the time down to about 10 seconds.  Then when this script
   # was modified to read in all the package info in andvance with one call to
   # the package manager, part of that info can be a list of "provides"
   # capabilities.  All of the packages "provides" list are used to "pre-load"
   # the affore mentioned cache. Testing resulted in a 95% cache hit rate and
   # a total time of less thant 1.5 seconds.

   #-------------------------------------------------------------------------
   def __init__( self, *args, **kwargs ):

      if 0 >= len(args) and 0 >= len(kwargs):
         return
      os_release = args[0]

      # Select a package manager.
      try:
         pm_cmd = self._os_package_managers[os_release['ID']]
      except:
         Console().error( "Can't choose a package manager to use" )
         sys.exit( 9 )

      # Ask the package manager for info about all packages.
      if False:#not CLI().progress:
         everything = proc.check_output( pm_cmd )
      else:
         bytes_to_go = self._sixteenK
         everything = b''
         pipe = proc.Popen( pm_cmd, stdout=proc.PIPE).stdout
         chunk = pipe.read( self._sixteenK )
         while 0 < len(chunk):
            everything += chunk
            bytes_to_go -= len(chunk)
            if 0 >= bytes_to_go:
               Console().tick()
               bytes_to_go = self._sixteenK
            chunk = pipe.read( self._sixteenK )
         Console().flush()
      if "bytes" == type(everything).__name__:
         everything = "".join(list(map(chr,everything)))
      everything_lines = everything.rstrip().split('\n')

      # Get uname info
      self._uname = unameT(*os.uname())

      # Scan the output of the package manager and load the package database.
      self._transmogrify_cache = {}
      self._pack_db = {}
      for line in everything_lines:
         fields = pkg_fieldsT(*line.split('\t'))

         # Get list of depends sans any conditions.
         if '' == fields.depends:
            if fields.name.startswith('gpb-pubkey'):
               # Not sure if this exception is needed.
               continue
            deplist = []
         else:
            deplist = fields.depends.rstrip(', \n').split(',')
         deplist = list(map(lambda x: re.sub(r' .*', '', x.lstrip()), deplist))

         if pm_cmd != self._rpm_cmd:
            database_key = fields.name

         else:
            # A little more extra work if RPM...
            database_key = re.sub(r'-[0-9]+:', '-', fields.name)

            # If the database_key is the name of package valid for our machine:
            if self._good_for_rpm(database_key):

               # Split the output list into two parts: depends and provides.
               delim_ndx = deplist.index( self._rpm_delimiter )
               provides_list = deplist[delim_ndx+1:]
               deplist[delim_ndx:] = []

               # Add the provides to the transmogrify cache.
               for provide in provides_list:
                  if provide not in self._transmogrify_cache:
                     self._transmogrify_cache[provide] = set()
                  self._transmogrify_cache[provide].add(database_key)

         # Add the package to the database.
         self._pack_db[database_key] = data_itemT(fields.version, deplist)

      # If RPM, change the transmogrify cache items from sets to lists and
      # set the transmogrify function to _capability2packages().
      if pm_cmd == self._rpm_cmd:
         self.transmogrify = self._capability2packages
         for k, v in self._transmogrify_cache.items():
            self._transmogrify_cache[k] = list(v)

      # Otherwise, set the transmogrify function to a pass-through.
      else:
         self.transmogrify = types.MethodType(lambda self, x: x, self)

      # If requested, dump out the database and the cache
      if CLI().verbose > Console().vlevels['dump_db']:
         for k, v in self.sorted_packages():
            Console().print_dbentry( k, v )
         for k, v in self.sorted_transmogrify_cache():
            Console().print_cap_pkglist( k, v )

   #-------------------------------------------------------------------------
   def getDepends( self, pkg ):
      return self.transmogrify(self[pkg].depends)

   #-------------------------------------------------------------------------
   def __getitem__( self, pkg ):
      return self._pack_db[pkg]

   #-------------------------------------------------------------------------
   def __setitem__( self, pkg, val ):
      self._pack_db[pkg] = val

   #-------------------------------------------------------------------------
   def __contains__( self, pkg ):
      return pkg in self._pack_db

   #-------------------------------------------------------------------------
   def _good_for_rpm( self, x ):
      """ Return True if the package 'x' is a keeper. """
      if self._rpm_delimiter in x:
         return True
      if x.startswith('rpmlib'):
         return False
      return x.endswith(self._uname.machine) or x.endswith('noarch')

   #-------------------------------------------------------------------------
   def _capability2packages( self, cap ):
      """ Find packages that implement the capability 'cap'.  If cap is a
          list, find the union of packages implementing those capabilities. """

      # Convert to list if single capability given
      if 'list' == type(cap).__name__:
         caps = cap
      else:
         caps = [ cap ]

      # Prime the pump.
      (result, todo, pack_lists, uninstalled) = ([], [], [], [])

      # Build two lists: 'results' is a list of capabilities found in our cache.
      #                  'todo' is a list of packages we need to query with rpm.
      for one_cap in caps:
         if one_cap.startswith('rpmlib'):
            continue

         # if this capability is cached, add its packages to the results.
         if one_cap in self._transmogrify_cache:
            new1 = self._transmogrify_cache[one_cap]
            result.append(new1)
         else:
            # if not cached, add to todo list.
            todo.append(one_cap)

      todo = list(set(todo))
      # Ask RPM for capabilities provided by each package in the todo list.
      if len(todo) > 0:

         # Because RPM can return more than one package that provides a given
         # capability, the only way to maintain corraspondance between
         # packages and capabilities is to force delimitors into RPM's output.
         todo_cooked = reduce(lambda r,v: r + [ self._rpm_delimiter, v],
                              todo[1:], todo[:1])
         todo_cooked.append(self._rpm_delimiter)

         # Run the RPM command.  Because of the forced delimitors, RPM will
         # give "no package provides" errors in its output and return with
         # a non-zero # return code.
         try:
            cmd_list =["rpm", "-q", "--whatprovides"] + todo_cooked
            packages = proc.check_output( cmd_list, stderr=proc.STDOUT)
         except proc.CalledProcessError as e:
            packages = e.output
         if "bytes" == type(packages).__name__:
            packages = "".join(list(map(chr,packages)))
         packages = packages.rstrip().split('\n')

         # Use the "no package provides @_@" messages to seperate the output
         # from RPM into a list of lists.  Each element of the top level list
         # corrasponds to a given capability and is a list of packages
         # providing that capabilits.
         a_pack_list = []
         for pack in packages:
            if self._rpm_delimiter in pack:
               pack_lists.append(list(set(filter(self._good_for_rpm,
                                                 a_pack_list))))
               a_pack_list = []
            else:
               a_pack_list.append(pack)

         assert len(todo) == len(pack_lists), \
                                    "todo and pack_list have different lengths"
         new_stuff = zip(todo,pack_lists)
         self._transmogrify_cache.update(new_stuff)
         for k,v in new_stuff:
            if [] == v:
               uninstalled.append(k)

      return list(chain.from_iterable(result + pack_lists)) + uninstalled

   #-------------------------------------------------------------------------
   def sorted_packages( self ):
      return sorted( self._pack_db.items(), key=lambda x: x[0] )

   #-------------------------------------------------------------------------
   def sorted_transmogrify_cache( self ):
      return sorted(self._transmogrify_cache.items(), key=lambda x: x[0] )

#End class PackageDB

#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#
# XQList -- Mosly a list, but act like a set with a few "extras".
#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#

class XQList ( list ):
   """ A list where each element is unique.  Also keeps track of appenders. """

   #-------------------------------------------------------------------------
   def __init__( self, *elems ):
      self.package_levels = {}
      super(XQList,self).__init__(set(*elems))
      if '' in super(XQList,self).__iter__():
         super(XQList,self).remove('')
      if None in super(XQList,self).__iter__():
         super(XQList,self).remove(None)
      self._uniq = defaultdict(dict)
      for x in self:
         self._uniq[x]['* * * Top Level * * *'] = True
         self.package_levels[x] = 0

   #-------------------------------------------------------------------------
   def sort( self ):
      super(XQList,self).sort()

   #-------------------------------------------------------------------------
   def append( self, val, source ):
      if 'list' != type(val).__name__:
         val = [ val ]
      for one_val in val:
         if one_val not in self._uniq:
            super(XQList,self).append( one_val )
         if source not in self._uniq[one_val]:
            self._uniq[one_val][source] = True
         if one_val not in self.package_levels:
            self.package_levels[one_val] = self.package_levels[source] + 1

   #-------------------------------------------------------------------------
   def max_package_level( self ):
      return max([v for v in self.package_levels.values()])

   #-------------------------------------------------------------------------
   def __setitem__():
      raise NotImplementedError

   #-------------------------------------------------------------------------
   def get_pkgs_that_require( self, pkg ):
      return self._uniq[pkg]

#End class XQList

#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#
# OS_Release -- Information from /etc/os-release, or just guess work.
#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#

@Singleton
class OS_Release ( dict ):
   """ Harvest information from /etc/os-release as a "mappable". """
Nn
   #-------------------------------------------------------------------------
   def guess( self ):
      if 0 == proc.call("which rpm >/dev/null", shell=True):
         return {'ID': 'rhel', 'VERSION_ID':'7.2', 'PRETTY_NAME' :
                 "(I'm guessing) Red Hat Enterprise Linux Server 7.2 (Maipo)"}
      if 0 == proc.call("which dpkg >/dev/null", shell=True):
         return {'ID': 'ubuntu', 'VERSION_ID' : '14.04', 'PRETTY_NAME' :
                 "(I'm guessing) Ubuntu 14.04.4 LTS"}

   #-------------------------------------------------------------------------
   def __init__( self ):
      try:
         f = open("/etc/os-release", "r")
         for line in f:
            line = line.rstrip()
            if re.match(r'^\s*$', line):
               continue
            mo = re.match(r'^(?P<name>[^=]*)=([\'"]?)(?P<value>.*)\2', line)
            d = mo.groupdict()
            self[d['name']] = d['value']
      except:
         self.clear()
         self.update(self.guess())

#End class XQList

#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#
# Application -- Input:   __init__()
#                Process: expand_depends()
#                Output:  output_results()
#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#

@Singleton
class Application (object):
   """ The top-most object... represents the whole applicaiton. """

   #-------------------------------------------------------------------------
   def __init__( self, exe_name, args ):

      #-- Initialize the Console interface.
      Console( os.getenv( "COLUMNS", 80 ) )

      #-- Process any command line arguments.
      CLI(exe_name, args)

      #-- Load the database
      PackageDB( OS_Release() )

      #-- Select which list of prereqs to use.
      self._prereqs = CLI().prereqs
      if None == self._prereqs:
         self._prereqs = StandardPrereqs( OS_Release() ).prereqs
      self.flat_count = len(self._prereqs)

   #-------------------------------------------------------------------------
   def expand_depends( self ):
      # Initialize xlist
      self.xlist = XQList(PackageDB().transmogrify(self._prereqs))

      # This is the heart of the whole script: for each package in the list,
      # add the packages it is dependent on to the list, if not already there,
      # and keep going until the end of the list is finialy reached.
      for pkg in self.xlist:
         if pkg not in PackageDB():
            PackageDB()[pkg] = data_itemT('', None)
            continue
         #if S-l-o-w, Console().tick()
         if not CLI().flat:
            self.xlist.append( PackageDB().getDepends(pkg), pkg)

   #-------------------------------------------------------------------------
   def output_results( self ):

      # Print out the results, package by package
      Console().flush()
      Console().max_name_len = len(max(self.xlist, key=len))
      self.xlist.sort()
      for pkg in self.xlist:
         Console().row_out(pkg, self.xlist )

      # If not not being "simple", print out the package counts.
      if not CLI().simple:
         print( "Packge count: %d" % len(self.xlist), end="" )
         if not CLI().flat:
            print( " (Initial count: %d, max depth: %d)" % (self.flat_count,
                       self.xlist.max_package_level()), end="" )
         print()

#End class Application

#############################################################################
#                                                                           #
#                               Main Program                                #
#                                                                           #
#############################################################################

if __name__ == "__main__":
   (exec_from, who_am_i) = os.path.split( sys.argv[0] )
   Application( who_am_i, sys.argv[1:] ) # One for the money,
   Application().expand_depends()        # Two for the show
   Application().output_results()        # Three to get ready,
   # Now go, cat go! (https://en.wikipedia.org/wiki/Blue_Suede_Shoes)

# vim:set sw=3 ts=3 ai et cc=78:
