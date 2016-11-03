#!/usr/bin/python
from __future__ import print_function
#
# Copyright(c) 2015-2016, Intel Corporation.
#
# Redistribution  and  use  in source  and  binary  forms,  with  or  without
# modification, are permitted provided that the following conditions are met:
#
# * Redistributions of  source code  must retain the  above copyright notice,
#   this list of conditions and the following disclaimer.
# * Redistributions in binary form must reproduce the above copyright notice,
#   this list of conditions and the following disclaimer in the documentation
#   and/or other materials provided with the distribution.
# * Neither the name  of Intel Corporation  nor the names of its contributors
#   may be used to  endorse or promote  products derived  from this  software
#   without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,  BUT NOT LIMITED TO,  THE
# IMPLIED WARRANTIES OF  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT  SHALL THE COPYRIGHT OWNER  OR CONTRIBUTORS BE
# LIABLE  FOR  ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR
# CONSEQUENTIAL  DAMAGES  (INCLUDING,  BUT  NOT LIMITED  TO,  PROCUREMENT  OF
# SUBSTITUTE GOODS OR SERVICES;  LOSS OF USE,  DATA, OR PROFITS;  OR BUSINESS
# INTERRUPTION)  HOWEVER CAUSED  AND ON ANY THEORY  OF LIABILITY,  WHETHER IN
# CONTRACT,  STRICT LIABILITY,  OR TORT  (INCLUDING NEGLIGENCE  OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,  EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#****************************************************************************
# @file aalinfo.py
# @brief Script to gather infomation about the AAL H/W and S/W environment.
# @ingroup
# @verbatim
# Accelerator Abstraction Layer Sample Application
#
#    This application is for example purposes only.
#    It is not intended to represent a model for developing
#    commercially-deployable applications.  It is designed to show working
#    examples of the AAL programming model and APIs.
#
# AUTHORS: Enno Luebbers, Intel Corporation
#          Paul Gilliam, Mindlance Consultant at Intel Corporation
#
# HISTORY:
# WHEN:          WHO:     WHAT:
# 08/04/2016     EL       Initial version. (was "peek.py")
# 08/12/2016     PG       Changed to "aalinfo" and added that functionality
# 09/23/2016     PG       Added support for multiple distros
# 09/27/2016     PG       Make it work with both python 2 and 3
# 09/27/2016     PG       Strip out "extra" stuff (like command line args)
#****************************************************************************

"""
Compile version information from the execution environment and from the FPGA.
"""

import mmap, sys, re, subprocess, os, os.path
from string import Template

#############################
#
#  todo list
#
#############################

#-----------------------------------------------------------
#  Generage a pattern that will match groups of hex digits.
def HexSegs ( *args ):
   ndx = 0
   retstr = ''
   ordA = ord('A')
   for seg in range(len(args)):
      if 'str' == type(args[seg]).__name__:
         retstr += args[seg]
      else:
         retstr += r'(?P<%c>[0-9a-fA-F]{%d})' % (chr(ordA+ndx), args[seg])
         ndx += 1
   return retstr

#------------------------------------------------------------------------
#  A little program to find the version number of a given python module.
def python_module_version( mod_name ):
   pyprog = "python" + str(sys.version_info.major)
   return """%(pyprog)s<<EOF
from __future__ import print_function
try:
   import %(mod)s
   print( %(mod)s.__version__ )
except:
   print( "%(mod)s module not installed." )
EOF""" % {'mod': mod_name, 'pyprog': pyprog }

#-----------------------------------------------------------------
# TodoList Class
class TodoList (object):

   # Data used to decide if we can skip todo items that deal with a pkg manager.
   _successful    = {}
   _pack_man_pat  = r'rpm|dpkg'
   _find_prereqs  = 'find_prereqs.py'
   _skip_item     = 'skip this todo item'

   # Regular Expressions used to extract version information.
   _libc_ver_pat  = r'stable release version (?P<ver>\d+(\.\d+)*)'
   _pkg_ver_pat   = r'^Version:\s+(?P<ver>[^\n]*) *$'
   _gnu_ver1_pat  = r'^(?P<name>[^(]*)\s*\((?P<lver>[^)]*)\)\s*(?P<sver>[^\n]*)$'
   _short_v2_pat  = r'^(?P<name>[^,]*),\s*version (?P<sver>\S*)'
   _long_v2_pat   = r'\s*\((?P<lver>[^)]*)\)[^\n]*$'
   _gnu_ver2_pat  = _short_v2_pat + _long_v2_pat
   _word_ver_pat  = r'(?P<name>[^\n]*?) *(?P<sver>\S*)$'
   _just_ver_pat  = r'(?P<sver>[^\n]*)$'
   _afu_id_pat    = r'0x' + HexSegs( 8, 4, 4, r'.*', 4, 12 )
   _bitstream_pat = r'0x' + HexSegs( 16, r'.*', 16 )

   _rhat_ver1_pat = r'^[^-]*-(?P<ver>[^-]*)-(?P<rel>.*)\.[^\n]*$'
   _rhat_ver2_pat = r'\(Red Hat (?P<ver>[^)]*)\)'
   _rhat_ver3_pat = r'version (?P<ver>[^-]*)-(?P<rel>[^ -]*)'
   _rhat_ver4_pat = r'\w+ (?P<ver>[^\n]*)\n'
   _rhat_ver5_pat = r'\w+\) (?P<ver>[^\n]*)\n'

   # Other Regular Expressions.
   _bdf_pat       = r'^(?P<bdf>[0-9a-fA-F]{2}:[0-9a-fA-F]{2}[^\n][0-9a-fA-F])'
   _dev_pat       = r'Intel.* (?P<dev>bcbd|bcc0)'
   _aaldev_pat    = _bdf_pat + '.*' + _dev_pat + ' *$'


   # Lists of what needs to be done and how to do it, one for each distro.
   # Each entry:
   #    <shell command(s)>, <regular expression(s)>, <template>

   _todo_list_redhat = [

      # --- AALSDK, etc. version info
      [ "aalscan" ],
      [ _find_prereqs ],

      # --- Compiler and Binary Utilities Versions
      [ "rpm -q binutils",    _rhat_ver1_pat,
            "Binutils Version (rpm): $ver  release: $rel" ],
      [ "rpm -q gcc",         _rhat_ver1_pat,
             "GCC Version (rpm): $ver  release: $rel" ],
      [ "gcc --version",      _rhat_ver2_pat,
             "GCC Version (--version): $ver" ],
      [ "g++ --version",      _rhat_ver2_pat,
             "G++ Version (--version): $ver" ],
      [ "ld --version",       _rhat_ver3_pat,
             "Binutils Version (--version): $ver  Release: $rel" ],
      [ "flex --version",     _rhat_ver4_pat,
             "Flex Version (--version): $ver"],
      [ "automake --version", _rhat_ver5_pat,
             "Automake Version (--version): $ver" ],
      [ "make --version",     _rhat_ver4_pat,
             "Make version (--version): $ver" ],
      [ "bash --version",     _rhat_ver3_pat,
             "Bash Version (--version): $ver" ],

      # --- Library Versions
      [ "/usr/lib/libc.so.6", _libc_ver_pat, "libc (direct): $ver" ],

      # --- Xeon+FPGA system versions
      [ "$peek $res_fname 0x40010 ; $peek $res_fname 0x40008",
            _afu_id_pat,
            "AFU_ID in PF PORT0: $A-$B-$C-$D-$E",
            "AFU_ID in PF PORT0: ** Xeon+FPGA not detected **" ],
      [ "$peek $res_fname 0x40068 ; $peek $res_fname 0x40060",
            _bitstream_pat,
            "BITSTREAM_ID: 0x$A   BITSTREAM_MD: 0x$B",
            "BITSTREAM ID & MD: ** Xeon+FPGA not detected **" ],

      # --- Python Stuff
      [ '$pyprog --version', _word_ver_pat,
            "Python Version: $sver" ],
      [ python_module_version( 'numpy' ), _just_ver_pat,
            "Python Numpy Version: $sver" ],
   ]

   _todo_list_suse = _todo_list_redhat

   _todo_list_ubuntu = [

      # --- AALSDK, etc. version info
      [ "aalscan" ],
      [ _find_prereqs ],

      # --- Compiler and Binary Utilities Versions
      [ "dpkg -s binutils",   _pkg_ver_pat,
            "Binutils Version (dpkg): $ver" ],
      [ "dpkg -s gcc",        _pkg_ver_pat,
            "GCC Version (dpkg): $ver" ],
      [ "gcc --version",      _gnu_ver1_pat,
            "GCC Version (--version): $sver" ],
      [ "dpkg -s g++",        _pkg_ver_pat,
            "G++ Version (dpkg): $ver" ],
      [ "g++ --version",      _gnu_ver1_pat,
            "G++ Version (--version): $sver" ],
      [ "ld --version",       _gnu_ver1_pat,
            "Binutils Version (--version): $sver" ],
      [ "flex --version",     _word_ver_pat,
            "Flex Version (--version): $sver" ],
      [ "automake --version", _word_ver_pat,
            "Automake Version (--version): $sver" ],
      [ "make --version",     _word_ver_pat,
            "Make version (--version): $sver" ],
      [ "bash --version",     _gnu_ver2_pat,
            "Bash Version (--version): $sver" ],

      # --- Library Versions
      [ "/lib/x86_64-linux-gnu/libc.so.6" , _libc_ver_pat,
           "libc (direct): $ver" ],

      # --- Xeon+FPGA system versions
      [ "$peek $res_fname 0x40010 ; $peek $res_fname 0x40008",
            _afu_id_pat,
            "AFU_ID in PF PORT0: $A-$B-$C-$D-$E",
            "AFU_ID in PF PORT0: ** Xeon+FPGA not detected **" ],
      [ "$peek $res_fname 0x40068 ; $peek $res_fname 0x40060",
            _bitstream_pat,
            "BITSTREAM_ID: 0x$A   BITSTREAM_MD: 0x$B",
            "BITSTREAM ID & MD: ** Xeon+FPGA not detected **" ],

      # --- Python Stuff
      [ '$pyprog --version', _word_ver_pat,
            "Python Version: $sver" ],
      [ python_module_version( 'numpy' ), _just_ver_pat,
            "Python Numpy Version: $sver" ],
   ]

   #-------------------------------------------
   #  Using info from 'lspci', find the file
   #  that must be mapped to access MMIO space.
   def find_resource_file( self ):
      devs = { 'bcbd' : "BDX-P", 'bcc0' : "SKX-P" }
      # Search for the aal PCIe device.
      lspci_out = subprocess.check_output( ["lspci"] )
      if "bytes" == type(lspci_out).__name__:
         lspci_out = "".join(map(chr,lspci_out))
      mo = re.search(TodoList._aaldev_pat, lspci_out, flags=re.MULTILINE)
      if not mo:
         print( "WARNING: No Xeon+FPGA PF found." )
         self.res_fname = None
         return
      print( "INFO: Found %s device at %s" % (devs[mo.group("dev")],
               mo.group('bdf')) )
      self.res_fname = "/sys/bus/pci/devices/0000:"
      self.res_fname += mo.group('bdf')
      self.res_fname += "/resource2"

   #-----------------------------
   #  Initialize
   def __init__( self, script_home ):

      self._script_home = script_home

      # Pick a distro.
      os_release = subprocess.check_output( ['cat', '/etc/os-release'] )
      if "bytes" == type(os_release).__name__:
         os_release = "".join(map(chr, os_release))
      if 'Ubuntu' in os_release:
         self._todo_list = TodoList._todo_list_ubuntu;
      elif 'Red Hat' in os_release:
         self._todo_list = TodoList._todo_list_redhat;
      elif 'SUSE' in os_release:
         self._todo_list = TodoList._todo_list_suse;
      else:
         print( "WARNING: Could not determain distro: guessing Red Hat" )
         self._todo_list = TodoList._todo_list_redhat;

      #  Find the FPGA resource file used to access MMIO
      self.find_resource_file();

      # Initialize the cmd_dict
      self.cmd_dict = { 'peek'     : './pcipeek.py',
                        'res_fname': self.res_fname,
                        'pyprog'   : "python" + str(sys.version_info.major) }

   #---------------------------------------
   #  'Run' a chore (single entry) in the todo list
   def do_one_chore ( self, *args ):

      # As a special case, see if this chore is doing a MMIO access.  If it
      # is and a Xeon+FPGA was not detecterd, return the altinate message.
      if '$peek' == args[0][:5] and None == self.cmd_dict['res_fname']:
         return args[3]

      # As another special case, only do a pkg manager based chore if the
      # TodoList._find_prereqs chore failed to run.
      if TodoList._find_prereqs in self._successful \
      and self._successful[TodoList._find_prereqs] \
      and re.match(TodoList._pack_man_pat, args[0]):
         return TodoList._skip_item

      # Templatize, expand from cmd_dict and run the script.
      cmd_line = Template(args[0]).safe_substitute(self.cmd_dict)
      self._successful[cmd_line] = False
      try:
         cmd_out = subprocess.check_output( cmd_line, stderr=subprocess.STDOUT,
                                            shell=True )
         if "bytes" == type(cmd_out).__name__:
            cmd_out = "".join(map(chr, cmd_out))
         self._successful[cmd_line] = True
      except subprocess.CalledProcessError as cpe:
         abspath = os.path.join(self._script_home, cmd_line)
         try:
            cmd_out = subprocess.check_output( abspath,
                                               stderr=subprocess.STDOUT,
                                               shell=True )
            self._successful[cmd_line] = True
         except:
            return '"%s" execution failed!' % (cmd_line)
      except:
         return '"%s" execution failed!' % (cmd_line)

      # If only one argument, then just return the output.
      if 1 == len(args):
         return cmd_out.rstrip()

      # There are at least two arguments: need at least three or error.
      if 3 > len(args):
         return 'ERROR: Regular expression(s) given, but no template.'

      # Use re.search to collect all the good stuff in a dict.
      mo = re.search(args[1], cmd_out, flags=re.MULTILINE|re.DOTALL)
      if None == mo:
         grp_dct = {}
      else:
         grp_dct = mo.groupdict()

      # Templitize the third argument, expand from the dict and return it.
      msg = "WARNING: Could not collect version information"
      bad_things = {'ver':msg, 'sver':msg, 'lver':msg, 'rel':''}
      return Template(args[2]).safe_substitute(bad_things, **grp_dct)

   #---------------------------------
   # Run each chore in the todo list
   def run( self ):
      for chore in self._todo_list:
         ans = self.do_one_chore(*chore)
         if ans != TodoList._skip_item:
            print( ans )



######################
#
#  Top level routines
#
######################

#-------------------
#  The main program.
def main( script_home, script_name, argv ):
   global cmd_dict

   # We should probably have an efective user id of ROOT (0).
   if 0 != os.geteuid():
      print( 'WARNING: You probably want to be "root" to run "%s".' % script_name )

   # Instantiat and run the 'todo' list.
   todo_list = TodoList(script_home)
   todo_list.run();

if __name__ == "__main__":
   (exec_from, who_am_i) = os.path.split( sys.argv[0] )
   main( exec_from, who_am_i, sys.argv[1:] )

# vim:set sw=3 ts=3 et ai cc=78: