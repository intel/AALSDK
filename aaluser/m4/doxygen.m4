dnl Copyright (c) 2013-2015, Intel Corporation
dnl
dnl Redistribution  and  use  in source  and  binary  forms,  with  or  without
dnl modification, are permitted provided that the following conditions are met:
dnl
dnl * Redistributions of  source code  must retain the  above copyright notice,
dnl   this list of conditions and the following disclaimer.
dnl * Redistributions in binary form must reproduce the above copyright notice,
dnl   this list of conditions and the following disclaimer in the documentation
dnl   and/or other materials provided with the distribution.
dnl * Neither the name  of Intel Corporation  nor the names of its contributors
dnl   may be used to  endorse or promote  products derived  from this  software
dnl   without specific prior written permission.
dnl
dnl THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
dnl AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,  BUT NOT LIMITED TO,  THE
dnl IMPLIED WARRANTIES OF  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
dnl ARE DISCLAIMED.  IN NO EVENT  SHALL THE COPYRIGHT OWNER  OR CONTRIBUTORS BE
dnl LIABLE  FOR  ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR
dnl CONSEQUENTIAL  DAMAGES  (INCLUDING,  BUT  NOT LIMITED  TO,  PROCUREMENT  OF
dnl SUBSTITUTE GOODS OR SERVICES;  LOSS OF USE,  DATA, OR PROFITS;  OR BUSINESS
dnl INTERRUPTION)  HOWEVER CAUSED  AND ON ANY THEORY  OF LIABILITY,  WHETHER IN
dnl CONTRACT,  STRICT LIABILITY,  OR TORT  (INCLUDING NEGLIGENCE  OR OTHERWISE)
dnl ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,  EVEN IF ADVISED OF THE
dnl POSSIBILITY OF SUCH DAMAGE.
dnl ****************************************************************************
dnl      Intel(R) Accelerator Abstraction Layer Library Software Developer
dnl         Kit (SDK)
dnl   Content:
dnl      Intel(R) Accelerator Abstraction Layer Library
dnl      m4/doxygen.m4
dnl   Author:
dnl      Tim Whisonant, Intel Corporation
dnl   History:
dnl      07/18/2013    TSW   Initial version
dnl ******************************************************************************

dnl # AALSDK_PROG_DOXYGEN(VERSION-PREREQ, [chm def html latex man perlmod pdf ps rtf xml])
AC_DEFUN([AALSDK_PROG_DOXYGEN], [
   AC_SUBST([DOX_DOXYFILE], [doc/Doxyfile])

   AC_ARG_VAR([DOXYGEN], [PATH to the doxygen executable.])
   AS_IF([test x"${DOXYGEN}" = x], [AC_PATH_PROG([DOXYGEN], [doxygen])])
   
   AC_ARG_VAR([PERL], [PATH to the perl executable.])
   AS_IF([test x"${PERL}" = x], [AC_PATH_PROG([PERL], [perl])])

   AC_SUBST([DOX_VERSION_PREREQ], [$1])

   _aal_use_doxygen=
   _aal_doxygen_version=
   AS_IF([test -n "${DOXYGEN}" && test -n "${PERL}"],
         [
          _aal_use_doxygen=yes
          AS_VAR_SET([_aal_doxygen_version], [`${DOXYGEN} --version`])
          AS_VERSION_COMPARE([$_aal_doxygen_version], [$1], [_aal_use_doxygen=no])
         ],
         [_aal_use_doxygen=no])

   AC_MSG_CHECKING([whether to use Doxygen])
   AC_CACHE_VAL([ac_cv_aal_use_doxygen], [ac_cv_aal_use_doxygen=${_aal_use_doxygen}])
   AC_MSG_RESULT([$ac_cv_aal_use_doxygen])

   AS_IF([test x"$ac_cv_aal_use_doxygen" = xyes], 
         [
          AC_SUBST([ABS_SRCDIR],                [`cd ${srcdir} && pwd`])
          AC_SUBST([DOX_PROJECT_LOGO],          [`cd ${srcdir} && pwd`/doc/intel_rgb_62.png])
          AC_SUBST([DOX_GENERATETODOLIST],      [NO])
          AC_SUBST([DOX_GENERATETESTLIST],      [NO])
          AC_SUBST([DOX_GENERATEBUGLIST],       [NO])
          AC_SUBST([DOX_GENERATEDEPRECATEDLIST],[YES])
          AC_SUBST([DOX_WARNINGS],              [YES])
          AC_SUBST([DOX_OUTPUT_DIRECTORY],      [doc/doxygen])
          AC_SUBST([DOX_GENERATE_TAGFILE],      [doc/doxygen/sdk.tag])
          AC_SUBST([DOX_STRIP_FROM_PATH],       [`cd ${srcdir} && pwd`])
          AC_SUBST([DOX_STRIP_FROM_INC_PATH],   [`cd ${srcdir} && pwd`/include])

          dnl Custom layout file works with 1.7.6 and greater.
          AC_MSG_CHECKING([whether to enable custom Doxygen layout])
          AS_VERSION_COMPARE([$_aal_doxygen_version], [1.7.6],
                             [ dnl less
                              AC_SUBST([DOX_LAYOUT_FILE], [''])
                              AC_MSG_RESULT([no])
                             ],
                             [ dnl equal
                              AC_SUBST([DOX_LAYOUT_FILE], [`cd ${srcdir} && pwd`/doc/DoxygenLayout.xml])
                              AC_MSG_RESULT([yes])
                             ],
                             [ dnl greater
                              AC_SUBST([DOX_LAYOUT_FILE], [`cd ${srcdir} && pwd`/doc/DoxygenLayout.xml])
                              AC_MSG_RESULT([yes])
                             ])

          AC_ARG_VAR([DOT], [PATH to the dot executable.])
          AS_IF([test x"${DOT}" = x], [AC_PATH_PROG([DOT], [dot])])
          AS_IF([test x"${DOT}" = x], [AC_SUBST([HAVE_DOT], [NO])], [AC_SUBST([HAVE_DOT], [YES])])

          AC_ARG_VAR([MSCGEN], [PATH to the mscgen executable.])
          AS_IF([test x"${MSCGEN}" = x], [AC_PATH_PROG([MSCGEN], [mscgen])])
      
          AC_ARG_VAR([DVIPS], [PATH to the dvips executable.])
          AS_IF([test x"${DVIPS}" = x], [AC_PATH_PROG([DVIPS], [dvips])])

          AC_ARG_VAR([LATEX], [PATH to the latex executable.])
          AS_IF([test x"${LATEX}" = x], [AC_PATH_PROG([LATEX], [latex])])

          AC_ARG_VAR([MAKEINDEX], [PATH to the makeindex executable.])
          AS_IF([test x"${MAKEINDEX}" = x], [AC_PATH_PROG([MAKEINDEX], [makeindex])])

          AC_ARG_VAR([PDFLATEX], [PATH to the pdflatex executable.])
          AS_IF([test x"${PDFLATEX}" = x], [AC_PATH_PROG([PDFLATEX], [pdflatex])])
          AS_IF([test x"${PDFLATEX}" = x], [AC_SUBST([DOX_USE_PDFLATEX],[NO])], [AC_SUBST([DOX_USE_PDFLATEX],[YES])])

          AC_PATH_PROG([EGREP], [egrep])

          m4_if(m4_index([$2], html), [-1], [_aal_doxygen_html=no], [_aal_doxygen_html=yes])
          AC_CACHE_VAL([ac_cv_aal_doxygen_html], [ac_cv_aal_doxygen_html=${_aal_doxygen_html}])

          AS_IF([test "x${ac_cv_aal_doxygen_html}" = xyes], [AC_SUBST([DOX_GENERATE_HTML], [YES])], [AC_SUBST([DOX_GENERATE_HTML], [NO])])

          m4_if(m4_index([$2], latex), [-1], [_aal_doxygen_latex=no], [_aal_doxygen_latex=yes])
          m4_if(m4_index([$2], pdf),   [-1], [_aal_doxygen_pdf=no],   [_aal_doxygen_pdf=yes])

          AS_IF([test "x${_aal_doxygen_pdf}" = xyes && test -n "${LATEX}" && test -n "${MAKEINDEX}" && test -n "${DVIPS}" && test -n "${EGREP}" && test -n "${PDFLATEX}"],
                [AC_SUBST([DOX_GENERATE_LATEX], [YES])],
                [
                 _aal_doxygen_pdf=no
                 AC_SUBST([DOX_GENERATE_LATEX], [NO])
                ])
          AC_CACHE_VAL([ac_cv_aal_doxygen_pdf], [ac_cv_aal_doxygen_pdf=${_aal_doxygen_pdf}])

          AS_IF([test "x${ac_cv_aal_doxygen_pdf}" = xyes], [_aal_doxygen_latex=yes],
                [
                 AS_IF([test "x${_aal_doxygen_latex}" = xyes && test -n "${LATEX}" && test -n "${MAKEINDEX}" && test -n "${DVIPS}" && test -n "${EGREP}"],
                       [AC_SUBST([DOX_GENERATE_LATEX], [YES])],
                       [
                        _aal_doxygen_latex=no
                        AC_SUBST([DOX_GENERATE_LATEX], [NO])
                       ])
                ])
          AC_CACHE_VAL([ac_cv_aal_doxygen_latex], [ac_cv_aal_doxygen_latex=${_aal_doxygen_latex}])

          AC_CACHE_VAL([ac_cv_aal_doxygen_formats],
                       [
                        ac_cv_aal_doxygen_formats=
                        test "x${_aal_doxygen_html}"  = xyes && ac_cv_aal_doxygen_formats="${ac_cv_aal_doxygen_formats} html"
                        test "x${_aal_doxygen_latex}" = xyes && ac_cv_aal_doxygen_formats="${ac_cv_aal_doxygen_formats} latex"
                        test "x${_aal_doxygen_pdf}"   = xyes && ac_cv_aal_doxygen_formats="${ac_cv_aal_doxygen_formats} pdf"
                       ])

          AC_SUBST([DOX_GENERATE_RTF], [NO])
          AC_SUBST([DOX_GENERATE_MAN], [NO])
          AC_SUBST([DOX_GENERATE_XML], [NO])
   ]) dnl # AS_IF([test x"$ac_cv_aal_use_doxygen" = xyes], 

   AM_CONDITIONAL([AAL_COND_DOXYGEN],      [test "x${ac_cv_aal_use_doxygen}" = xyes && test "x${ac_cv_aal_doxygen_formats}" != x])
   AM_CONDITIONAL([AAL_COND_DOXYGEN_HTML], [test "x${ac_cv_aal_doxygen_html}" = xyes])
   AM_CONDITIONAL([AAL_COND_DOXYGEN_LATEX],[test "x${ac_cv_aal_doxygen_latex}" = xyes])
   AM_CONDITIONAL([AAL_COND_DOXYGEN_PDF],  [test "x${ac_cv_aal_doxygen_pdf}" = xyes])
]) dnl # AALSDK_PROG_DOXYGEN


