dnl # AALSDK_PROG_GCOV
dnl # ---
AC_DEFUN([AALSDK_PROG_GCOV], [
   AC_ARG_VAR([GCOV], [GNU coverage testing tool])
   AS_IF([test "x${GCOV}" = x], [AC_PATH_PROG([GCOV], [gcov])])
])

dnl # AALSDK_PROG_LCOV
dnl # ---
AC_DEFUN([AALSDK_PROG_LCOV], [
   AC_ARG_VAR([LCOV], [a graphical GCOV front end])
   AS_IF([test "x${LCOV}" = x], [AC_PATH_PROG([LCOV], [lcov])])
])

dnl # AALSDK_PROG_GENHTML
dnl # ---
AC_DEFUN([AALSDK_PROG_GENHTML], [
   AC_ARG_VAR([GENHTML], [generate HTML view from LCOV coverage data files])
   AS_IF([test "x${GENHTML}" = x], [AC_PATH_PROG([GENHTML], [genhtml])])
])

dnl # WITH_GCOV_ANALYSIS
dnl # ---
AC_DEFUN([WITH_GCOV_ANALYSIS], [
   AALSDK_PROG_GCOV
   AALSDK_PROG_LCOV
   AALSDK_PROG_GENHTML

   _my_gcov_analysis=no
   _my_gcov_reason=
   AC_ARG_WITH([gcov],
               [AS_HELP_STRING([--with-gcov], [instrument for gcov @<:@default=no@:>@])],
               [AALSDK_YES_NO([${withval}], [_my_gcov_analysis], [], [],
                              [AC_MSG_ERROR([Invalid value '${withval}' for --with-gcov])])
               ])

   AC_MSG_CHECKING([whether to instrument for gcov])
   AS_IF([test "x${_my_gcov_analysis}" != xyes ], [_my_gcov_reason=' (not requested)'],
         [test "x${GCOV}" = x], [
          _my_gcov_analysis=no
          _my_gcov_reason=' (gcov not found)'
         ],
         [test "x${LCOV}" = x], [
          _my_gcov_analysis=no
          _my_gcov_reason=' (lcov not found)'
         ],
         [test "x${GENHTML}" = x], [
          _my_gcov_analysis=no
          _my_gcov_reason=' (genhtml not found)'
         ])

   AC_CACHE_VAL([ac_cv_gcov_analysis], [ac_cv_gcov_analysis="${_my_gcov_analysis}"])
   AC_MSG_RESULT([${_my_gcov_analysis}${_my_gcov_reason}])

   AM_CONDITIONAL([COND_WITH_GCOV], [test "x${ac_cv_gcov_analysis}" = xyes])
   AM_COND_IF([COND_WITH_GCOV],
              [
               GCOV_CFLAGS='-g -O0 --coverage'
               GCOV_CXXFLAGS='-g -O0 --coverage'
               GCOV_LDFLAGS=--coverage
               GCOV_LIBS=-lgcov
             ])
   AC_SUBST([GCOV_CFLAGS],   [${GCOV_CFLAGS}])
   AC_SUBST([GCOV_CXXFLAGS], [${GCOV_CXXFLAGS}])
   AC_SUBST([GCOV_LDFLAGS],  [${GCOV_LDFLAGS}])
   AC_SUBST([GCOV_LIBS],     [${GCOV_LIBS}])
   AC_SUBST([WITH_GCOV],     [${ac_cv_gcov_analysis}])
])

