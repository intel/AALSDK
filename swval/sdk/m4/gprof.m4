dnl # AALSDK_PROG_GPROF
dnl # ---
AC_DEFUN([AALSDK_PROG_GPROF], [
   AC_ARG_VAR([GPROF], [display call graph profile data])
   AS_IF([test "x${GPROF}" = x], [AC_PATH_PROG([GPROF], [gprof])])
])

dnl # WITH_GPROF_PROFILING
dnl # ---
AC_DEFUN([WITH_GPROF_PROFILING], [
   AALSDK_PROG_GPROF
   AALSDK_PROG_CXXFILT

   _my_gprof_profiling=no
   _my_gprof_reason=
   AC_ARG_WITH([gprof],
               [AS_HELP_STRING([--with-gprof], [instrument for gprof @<:@default=no@:>@])],
               [AALSDK_YES_NO([${withval}], [_my_gprof_profiling], [], [],
                              [AC_MSG_ERROR([Invalid value '${withval}' for --with-gprof])])
               ])

   AC_MSG_CHECKING([whether to instrument for gprof])
   AS_IF([test "x${_my_gprof_profiling}" != xyes], [_my_gprof_reason=' (not requested)'],
         [test "x${GPROF}" = x], [
           _my_gprof_profiling=no
           _my_gprof_reason=' (gprof not found)'
         ],
         [test "x${CXXFILT}" = x], [
           _my_gprof_profiling=no
           _my_gprof_reason=' (c++filt not found)'
         ])
   AC_CACHE_VAL([ac_cv_gprof_profiling], [ac_cv_gprof_profiling="${_my_gprof_profiling}"])
   AC_MSG_RESULT([${_my_gprof_profiling}${_my_gprof_reason}])

   AM_CONDITIONAL([COND_WITH_GPROF], [test "x${ac_cv_gprof_profiling}" = xyes])
   AM_COND_IF([COND_WITH_GPROF],
              [
               GPROF_CFLAGS='-g -O2 -pg'
               GPROF_CXXFLAGS='-g -O2 -pg'
               GPROF_LDFLAGS=-pg
              ])
   AC_SUBST([GPROF_CFLAGS],   [${GPROF_CFLAGS}])
   AC_SUBST([GPROF_CXXFLAGS], [${GPROF_CXXFLAGS}])
   AC_SUBST([GPROF_LDFLAGS],  [${GPROF_LDFLAGS}])
   AC_SUBST([WITH_GPROF],     [${ac_cv_gprof_profiling}])
])

