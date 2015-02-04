dnl INTEL CONFIDENTIAL - For Intel Internal Use Only

dnl # AALSDK_PROG_VALGRIND
dnl # ---
AC_DEFUN([AALSDK_PROG_VALGRIND], [
   AC_ARG_VAR([VALGRIND], [a suite of tools for debugging and profiling programs])
   AS_IF([test "x${VALGRIND}" = x], [AC_PATH_PROG([VALGRIND], [valgrind])])
])

dnl # WITH_VALGRIND_ANALYSIS
dnl # ---
AC_DEFUN([WITH_VALGRIND_ANALYSIS], [
   AALSDK_PROG_VALGRIND

   _my_valgrind_tool=
   _my_valgrind_reason=
   AC_ARG_WITH([valgrind],
               [AS_HELP_STRING([--with-valgrind], [memcheck, cachegrind, callgrind, helgrind, drd, massif, dhat, sgcheck @<:@default=none@:>@])],
               [AS_CASE([${withval}],
                        [memcheck],   [_my_valgrind_tool="${withval}"],
                        [cachegrind], [_my_valgrind_tool="${withval}"],
                        [callgrind],  [_my_valgrind_tool="${withval}"],
                        [helgrind],   [_my_valgrind_tool="${withval}"],
                        [drd],        [_my_valgrind_tool="${withval}"],
                        [massif],     [_my_valgrind_tool="${withval}"],
                        [dhat],       [_my_valgrind_tool="${withval}"],
                        [sgcheck],    [_my_valgrind_tool="${withval}"],
                        [AC_MSG_ERROR([Invalid value '${withval}' for --with-valgrind])])],
               [_my_valgrind_tool=none])

   AC_MSG_CHECKING([valgrind analysis])
   AS_IF([test "x${_my_valgrind_tool}" = xnone], [_my_valgrind_reason=' (not requested)'],
         [test "x${VALGRIND}" = x], [
          _my_valgrind_tool=none
          _my_valgrind_reason=' (valgrind not found)'
         ])

   AC_CACHE_VAL([ac_cv_valgrind_analysis], [ac_cv_valgrind_analysis="${_my_valgrind_tool}"])
   AC_MSG_RESULT([${ac_cv_valgrind_analysis}${_my_valgrind_reason}])

   AM_CONDITIONAL([COND_WITH_VALGRIND], [test "x${ac_cv_valgrind_analysis}" != xnone])
   AS_CASE([${ac_cv_valgrind_analysis}],
           [memcheck], [ dnl disable optimizations and inlining
            VALGRIND_CFLAGS='-g -O0 -fno-inline'
            VALGRIND_CXXFLAGS='-g -O0 -fno-inline'
           ],
           [cachegrind], [ dnl compile normally
            VALGRIND_CFLAGS='-g -O2'
            VALGRIND_CXXFLAGS='-g -O2'
           ],
           [callgrind], [ dnl compile normally
            VALGRIND_CFLAGS='-g -O2'
            VALGRIND_CXXFLAGS='-g -O2'
           ],
           [helgrind], [ dnl compile normally
            VALGRIND_CFLAGS='-g -O2'
            VALGRIND_CXXFLAGS='-g -O2'
           ],
           [drd], [ dnl use -O1
            VALGRIND_CFLAGS='-g -O1'
            VALGRIND_CXXFLAGS='-g -O1'
           ],
           [massif], [ dnl compile normally
            VALGRIND_CFLAGS='-g -O2'
            VALGRIND_CXXFLAGS='-g -O2'
           ],
           [dhat], [ dnl compile normally
            VALGRIND_CFLAGS='-g -O2'
            VALGRIND_CXXFLAGS='-g -O2'
           ],
           [sgcheck], [ dnl compile normally
            VALGRIND_CFLAGS='-g -O2'
            VALGRIND_CXXFLAGS='-g -O2'
           ])

   AC_SUBST([VALGRIND_CFLAGS],   [${VALGRIND_CFLAGS}])
   AC_SUBST([VALGRIND_CXXFLAGS], [${VALGRIND_CXXFLAGS}])
   AC_SUBST([WITH_VALGRIND],     [${ac_cv_valgrind_analysis}])
])

