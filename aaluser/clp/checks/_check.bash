#!/bin/bash
## Copyright(c) 2012-2016, Intel Corporation
##
## Redistribution  and  use  in source  and  binary  forms,  with  or  without
## modification, are permitted provided that the following conditions are met:
##
## * Redistributions of  source code  must retain the  above copyright notice,
##   this list of conditions and the following disclaimer.
## * Redistributions in binary form must reproduce the above copyright notice,
##   this list of conditions and the following disclaimer in the documentation
##   and/or other materials provided with the distribution.
## * Neither the name  of Intel Corporation  nor the names of its contributors
##   may be used to  endorse or promote  products derived  from this  software
##   without specific prior written permission.
##
## THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
## AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,  BUT NOT LIMITED TO,  THE
## IMPLIED WARRANTIES OF  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
## ARE DISCLAIMED.  IN NO EVENT  SHALL THE COPYRIGHT OWNER  OR CONTRIBUTORS BE
## LIABLE  FOR  ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR
## CONSEQUENTIAL  DAMAGES  (INCLUDING,  BUT  NOT LIMITED  TO,  PROCUREMENT  OF
## SUBSTITUTE GOODS OR SERVICES;  LOSS OF USE,  DATA, OR PROFITS;  OR BUSINESS
## INTERRUPTION)  HOWEVER CAUSED  AND ON ANY THEORY  OF LIABILITY,  WHETHER IN
## CONTRACT,  STRICT LIABILITY,  OR TORT  (INCLUDING NEGLIGENCE  OR OTHERWISE)
## ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,  EVEN IF ADVISED OF THE
## POSSIBILITY OF SUCH DAMAGE.
##****************************************************************************
##     Intel(R) Accelerator Abstraction Layer Library Software Developer
##        Kit (SDK)
##  Content:
##     Intel(R) Accelerator Abstraction Layer Library
##        memtest/checks/_check.bash
##  Author:
##     Tim Whisonant, Intel Corporation
##  History:
##     11/27/2012    TSW   Initial version
##******************************************************************************

declare -r SCANNER="${PWD}/scanner"

non_option_checks() {
${SCANNER} -
${SCANNER} - -
${SCANNER} --
${SCANNER} -- --
${SCANNER} - --
${SCANNER} -- -

${SCANNER} ---
${SCANNER} - ---
${SCANNER} --- -
${SCANNER} -- ---
${SCANNER} --- --
${SCANNER} --- ---

${SCANNER} -a -
${SCANNER} - -a
${SCANNER} -b --
${SCANNER} -- -b
${SCANNER} --- -c
${SCANNER} -c ---

${SCANNER} -ax -
${SCANNER} - -ax
${SCANNER} -by --
${SCANNER} -- -by
${SCANNER} --- -cz
${SCANNER} -cz ---

${SCANNER} a-
${SCANNER} a- -
${SCANNER} - a-
${SCANNER} b-- --
${SCANNER} -- b--
${SCANNER} -c-
${SCANNER} -c- -
${SCANNER} - -c-

${SCANNER} a
${SCANNER} - a
${SCANNER} a -
${SCANNER} -- b
${SCANNER} b --
${SCANNER} --- c
${SCANNER} c ---
${SCANNER} d d

${SCANNER} ee
${SCANNER} - ee
${SCANNER} ee -
${SCANNER} -- ff
${SCANNER} ff --
${SCANNER} --- gg
${SCANNER} gg ---
${SCANNER} hh hh
}

create_non_option_checker_compare() {
   cat >"$1" <<EOF
dash only
dash only
dash only
dash dash only
dash dash only
dash dash only
dash only
dash dash only
dash dash only
dash only
EOF
   cat >>"$1" <<EOF
Non-option: ---
dash only
Non-option: ---
Non-option: ---
dash only
dash dash only
Non-option: ---
Non-option: ---
dash dash only
Non-option: ---
Non-option: ---
EOF
   cat >>"$1" <<EOF
*nix short option: -a
dash only
dash only
*nix short option: -a
*nix short option: -b
dash dash only
dash dash only
*nix short option: -b
Non-option: ---
*nix short option: -c
*nix short option: -c
Non-option: ---
EOF
   cat >>"$1" <<EOF
*nix short option: -a
*nix short option: -x
dash only
dash only
*nix short option: -a
*nix short option: -x
*nix short option: -b
*nix short option: -y
dash dash only
dash dash only
*nix short option: -b
*nix short option: -y
Non-option: ---
*nix short option: -c
*nix short option: -z
*nix short option: -c
*nix short option: -z
Non-option: ---
EOF
   cat >>"$1" <<EOF
Non-option: a-
Non-option: a-
dash only
dash only
Non-option: a-
Non-option: b--
dash dash only
dash dash only
Non-option: b--
*nix short option: -c
*nix short option: --
*nix short option: -c
*nix short option: --
dash only
dash only
*nix short option: -c
*nix short option: --
EOF
   cat >>"$1" <<EOF
Non-option: a
dash only
Non-option: a
Non-option: a
dash only
dash dash only
Non-option: b
Non-option: b
dash dash only
Non-option: ---
Non-option: c
Non-option: c
Non-option: ---
Non-option: d
Non-option: d
EOF
   cat >>"$1" <<EOF
Non-option: ee
dash only
Non-option: ee
Non-option: ee
dash only
dash dash only
Non-option: ff
Non-option: ff
dash dash only
Non-option: ---
Non-option: gg
Non-option: gg
Non-option: ---
Non-option: hh
Non-option: hh
EOF
}


nix_short_checks() {
${SCANNER} -a
${SCANNER} -a -b
${SCANNER} -b x

${SCANNER} -a -b x
${SCANNER} -b x -a

${SCANNER} -a -b x -c
${SCANNER} -a -c -b x
${SCANNER} -b x -a -c

${SCANNER} -a -efg -b x -c
${SCANNER} -efg -b x -c -a
${SCANNER} -b x -c -a -efg
${SCANNER} -c -a -efg -b x

${SCANNER} -b x -a -efg -c
${SCANNER} -a -efg -c -b x
${SCANNER} -efg -c -b x -a
${SCANNER} -c -b x -a -efg
}

create_nix_short_checker_compare() {
   cat >"$1" <<EOF
*nix short option: -a
*nix short option: -a
*nix short option: -b
*nix short option/value: -b x
EOF
   cat >>"$1" <<EOF
*nix short option: -a
*nix short option/value: -b x
*nix short option/value: -b x
*nix short option: -a
EOF
   cat >>"$1" <<EOF
*nix short option: -a
*nix short option/value: -b x
*nix short option: -c
*nix short option: -a
*nix short option: -c
*nix short option/value: -b x
*nix short option/value: -b x
*nix short option: -a
*nix short option: -c
EOF
   cat >>"$1" <<EOF
*nix short option: -a
*nix short option: -e
*nix short option: -f
*nix short option: -g
*nix short option/value: -b x
*nix short option: -c
*nix short option: -e
*nix short option: -f
*nix short option: -g
*nix short option/value: -b x
*nix short option: -c
*nix short option: -a
*nix short option/value: -b x
*nix short option: -c
*nix short option: -a
*nix short option: -e
*nix short option: -f
*nix short option: -g
*nix short option: -c
*nix short option: -a
*nix short option: -e
*nix short option: -f
*nix short option: -g
*nix short option/value: -b x
EOF
   cat >>"$1" <<EOF
*nix short option/value: -b x
*nix short option: -a
*nix short option: -e
*nix short option: -f
*nix short option: -g
*nix short option: -c
*nix short option: -a
*nix short option: -e
*nix short option: -f
*nix short option: -g
*nix short option: -c
*nix short option/value: -b x
*nix short option: -e
*nix short option: -f
*nix short option: -g
*nix short option: -c
*nix short option/value: -b x
*nix short option: -a
*nix short option: -c
*nix short option/value: -b x
*nix short option: -a
*nix short option: -e
*nix short option: -f
*nix short option: -g
EOF
}


nix_long_checks() {

${SCANNER} --/
${SCANNER} --/ /x/y/z
${SCANNER} --// /x/y/z
${SCANNER} --/=/x/y/z
${SCANNER} --//=/x/y/z

${SCANNER} --
${SCANNER} --a --
${SCANNER} -- --a

${SCANNER} --a
${SCANNER} --a --b
${SCANNER} --b x
${SCANNER} --b=x

${SCANNER} --a --b x
${SCANNER} --a --b=x
${SCANNER} --b x --a
${SCANNER} --b=x --a

${SCANNER} --a --b x --c
${SCANNER} --a --b=x --c
${SCANNER} --a --c --b x
${SCANNER} --a --c --b=x
${SCANNER} --b x --a --c
${SCANNER} --b=x --a --c

${SCANNER} --a --efg --b x --c
${SCANNER} --a --efg --b=x --c
${SCANNER} --efg --b x --c --a
${SCANNER} --efg --b=x --c --a
${SCANNER} --b x --c --a --efg
${SCANNER} --b=x --c --a --efg
${SCANNER} --c --a --efg --b x
${SCANNER} --c --a --efg --b=x

${SCANNER} --b x --a --efg --c
${SCANNER} --b=x --a --efg --c
${SCANNER} --a --efg --c --b x
${SCANNER} --a --efg --c --b=x
${SCANNER} --efg --c --b x --a
${SCANNER} --efg --c --b=x --a
${SCANNER} --c --b x --a --efg
${SCANNER} --c --b=x --a --efg

${SCANNER} --b xy --a --efg yz --c
${SCANNER} --b=xy --a --efg=yz --c
${SCANNER} --c --b xy --a --efg yz
${SCANNER} --c --b=xy --a --efg=yz

${SCANNER} --test=ab --pattern=incr --src-phys=0x01234567890123456789 --KB=4
${SCANNER} --test=ab --pattern=incr --KB=4 --src-phys=0x01234567890123456789
}

create_nix_long_checker_compare() {
   cat >"$1" <<EOF
*nix long option: --/
*nix long option/value: --/ /x/y/z
*nix long option/value: --// /x/y/z
*nix long option/value: --/ /x/y/z
*nix long option/value: --// /x/y/z
EOF
   cat >>"$1" <<EOF
dash dash only
*nix long option: --a
dash dash only
dash dash only
*nix long option: --a
EOF
   cat >>"$1" <<EOF
*nix long option: --a
*nix long option: --a
*nix long option: --b
*nix long option/value: --b x
*nix long option/value: --b x
EOF
   cat >>"$1" <<EOF
*nix long option: --a
*nix long option/value: --b x
*nix long option: --a
*nix long option/value: --b x
*nix long option/value: --b x
*nix long option: --a
*nix long option/value: --b x
*nix long option: --a
EOF
   cat >>"$1" <<EOF
*nix long option: --a
*nix long option/value: --b x
*nix long option: --c
*nix long option: --a
*nix long option/value: --b x
*nix long option: --c
*nix long option: --a
*nix long option: --c
*nix long option/value: --b x
*nix long option: --a
*nix long option: --c
*nix long option/value: --b x
*nix long option/value: --b x
*nix long option: --a
*nix long option: --c
*nix long option/value: --b x
*nix long option: --a
*nix long option: --c
EOF
   cat >>"$1" <<EOF
*nix long option: --a
*nix long option: --efg
*nix long option/value: --b x
*nix long option: --c
*nix long option: --a
*nix long option: --efg
*nix long option/value: --b x
*nix long option: --c
*nix long option: --efg
*nix long option/value: --b x
*nix long option: --c
*nix long option: --a
*nix long option: --efg
*nix long option/value: --b x
*nix long option: --c
*nix long option: --a
*nix long option/value: --b x
*nix long option: --c
*nix long option: --a
*nix long option: --efg
*nix long option/value: --b x
*nix long option: --c
*nix long option: --a
*nix long option: --efg
*nix long option: --c
*nix long option: --a
*nix long option: --efg
*nix long option/value: --b x
*nix long option: --c
*nix long option: --a
*nix long option: --efg
*nix long option/value: --b x
EOF
   cat >>"$1" <<EOF
*nix long option/value: --b x
*nix long option: --a
*nix long option: --efg
*nix long option: --c
*nix long option/value: --b x
*nix long option: --a
*nix long option: --efg
*nix long option: --c
*nix long option: --a
*nix long option: --efg
*nix long option: --c
*nix long option/value: --b x
*nix long option: --a
*nix long option: --efg
*nix long option: --c
*nix long option/value: --b x
*nix long option: --efg
*nix long option: --c
*nix long option/value: --b x
*nix long option: --a
*nix long option: --efg
*nix long option: --c
*nix long option/value: --b x
*nix long option: --a
*nix long option: --c
*nix long option/value: --b x
*nix long option: --a
*nix long option: --efg
*nix long option: --c
*nix long option/value: --b x
*nix long option: --a
*nix long option: --efg
EOF
   cat >>"$1" <<EOF
*nix long option/value: --b xy
*nix long option: --a
*nix long option/value: --efg yz
*nix long option: --c
*nix long option/value: --b xy
*nix long option: --a
*nix long option/value: --efg yz
*nix long option: --c
*nix long option: --c
*nix long option/value: --b xy
*nix long option: --a
*nix long option/value: --efg yz
*nix long option: --c
*nix long option/value: --b xy
*nix long option: --a
*nix long option/value: --efg yz
EOF
   cat >>"$1" <<EOF
*nix long option/value: --test ab
*nix long option/value: --pattern incr
*nix long option/value: --src-phys 0x01234567890123456789
*nix long option/value: --KB 4
*nix long option/value: --test ab
*nix long option/value: --pattern incr
*nix long option/value: --KB 4
*nix long option/value: --src-phys 0x01234567890123456789
EOF
}


win_short_checks() {
${SCANNER} /
${SCANNER} /a /
${SCANNER} / /a

${SCANNER} /a
${SCANNER} /a /b
${SCANNER} /b x

${SCANNER} /a /b x
${SCANNER} /b x /a

${SCANNER} /a /b x /c
${SCANNER} /a /c /b x
${SCANNER} /b x /a /c
}

create_win_short_checker_compare() {
   cat >"$1" <<EOF
Non-option: /
Win short option: /a
Non-option: /
Non-option: /
Win short option: /a
EOF
   cat >>"$1" <<EOF
Win short option: /a
Win short option: /a
Win short option: /b
Win short option/value: /b x
EOF
   cat >>"$1" <<EOF
Win short option: /a
Win short option/value: /b x
Win short option/value: /b x
Win short option: /a
EOF
   cat >>"$1" <<EOF
Win short option: /a
Win short option/value: /b x
Win short option: /c
Win short option: /a
Win short option: /c
Win short option/value: /b x
Win short option/value: /b x
Win short option: /a
Win short option: /c
EOF
}


win_long_checks() {
${SCANNER} //
${SCANNER} /az //
${SCANNER} // /az

${SCANNER} ///
${SCANNER} /def ///
${SCANNER} /// /def

${SCANNER} /az
${SCANNER} /az /by
${SCANNER} /by x

${SCANNER} /az /by x
${SCANNER} /by x /az

${SCANNER} /az /by x /cx
${SCANNER} /az /cx /by x
${SCANNER} /by x /az /cx

${SCANNER} /az /dw /by x /cx
${SCANNER} /dw /by x /cx /az
${SCANNER} /by x /cx /az /dw
${SCANNER} /cx /az /dw /by x
}

create_win_long_checker_compare() {
   cat >"$1" <<EOF
Non-option: //
Win long option: /az
Non-option: //
Non-option: //
Win long option: /az
EOF
   cat >>"$1" <<EOF
Non-option: ///
Win long option: /def
Non-option: ///
Non-option: ///
Win long option: /def
EOF
   cat >>"$1" <<EOF
Win long option: /az
Win long option: /az
Win long option: /by
Win long option/value: /by x
EOF
   cat >>"$1" <<EOF
Win long option: /az
Win long option/value: /by x
Win long option/value: /by x
Win long option: /az
EOF
   cat >>"$1" <<EOF
Win long option: /az
Win long option/value: /by x
Win long option: /cx
Win long option: /az
Win long option: /cx
Win long option/value: /by x
Win long option/value: /by x
Win long option: /az
Win long option: /cx
EOF
   cat >>"$1" <<EOF
Win long option: /az
Win long option: /dw
Win long option/value: /by x
Win long option: /cx
Win long option: /dw
Win long option/value: /by x
Win long option: /cx
Win long option: /az
Win long option/value: /by x
Win long option: /cx
Win long option: /az
Win long option: /dw
Win long option: /cx
Win long option: /az
Win long option: /dw
Win long option/value: /by x
EOF
}

if true; then
   printf "Creating non-option tests..\n"
   create_non_option_checker_compare './non_option_checks_compare' 
   non_option_checks >'./non_option_checks'
   diff './non_option_checks_compare' './non_option_checks' || exit 1
fi

if true; then
   printf "Creating *nix short option tests..\n"
   create_nix_short_checker_compare './nix_short_checks_compare' 
   nix_short_checks >'./nix_short_checks'
   diff './nix_short_checks_compare' './nix_short_checks' || exit 1
fi

if true; then
   printf "Creating *nix long option tests..\n"
   create_nix_long_checker_compare './nix_long_checks_compare' 
   nix_long_checks >'./nix_long_checks'
   diff './nix_long_checks_compare' './nix_long_checks' || exit 1
fi

if false; then
   printf "Creating Windows short option tests..\n"
   create_win_short_checker_compare './win_short_checks_compare' 
   win_short_checks >'./win_short_checks'
   diff './win_short_checks_compare' './win_short_checks' || exit 1
fi

if false; then
   printf "Creating Windows long option tests..\n"
   create_win_long_checker_compare './win_long_checks_compare' 
   win_long_checks >'./win_long_checks'
   diff './win_long_checks_compare' './win_long_checks' || exit 1
fi

exit 0
