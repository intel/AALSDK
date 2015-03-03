dnl INTEL CONFIDENTIAL - For Intel Internal Use Only

dnl # AALSDK_PROG_GDB
dnl # ---
AC_DEFUN([AALSDK_PROG_GDB], [
   AC_ARG_VAR([GDB], [GNU debugger])
   AS_IF([test "x${GDB}" = x], [AC_PATH_PROG([GDB], [gdb])])
   AM_CONDITIONAL([COND_HAVE_GDB], [test "x${GDB}" != x])

   _my_gdb_level=
   _my_want_gdb=yes
   AC_ARG_WITH([gdb],
               [AS_HELP_STRING([--with-gdb], [set gdb debug info level @<:@default=3@:>@])],
               [AS_CASE([${withval}],
                        [0], [_my_gdb_level="${withval}"],
                        [1], [_my_gdb_level="${withval}"],
                        [2], [_my_gdb_level="${withval}"],
                        [3], [_my_gdb_level="${withval}"],
                        [yes], [_my_gdb_level=3],
                        [AC_MSG_ERROR([Invalid value '${withval}' for --with-gdb])])
               ],
               [
                _my_gdb_level=3
                _my_want_gdb=no
               ])

   AC_CACHE_CHECK([whether gdb was requested], [ac_cv_want_gdb], [ac_cv_want_gdb="${_my_want_gdb}"])
   AM_CONDITIONAL([COND_WANT_GDB], [test "x${ac_cv_want_gdb}" = xyes])

   AM_COND_IF([COND_HAVE_GDB],
              [ dnl transform any -g to -ggdb${lvl}
               AC_CACHE_CHECK([gdb debug info level], [ac_cv_gdb_dbg_level], [ac_cv_gdb_dbg_level="${_my_gdb_level}"])
               GDB_CFLAGS=`echo $CFLAGS | sed -re "s,-g([[[:space:]]]|$),-ggdb${ac_cv_gdb_dbg_level}\1,"`
               GDB_CXXFLAGS=`echo $CXXFLAGS | sed -re "s,-g([[[:space:]]]|$),-ggdb${ac_cv_gdb_dbg_level}\1,"`
              ])

   AC_SUBST([GDB_CFLAGS],   [${GDB_CFLAGS}])
   AC_SUBST([GDB_CXXFLAGS], [${GDB_CXXFLAGS}])
])

dnl # AALSDK_PROG_GDBTUI
dnl # ---
AC_DEFUN([AALSDK_PROG_GDBTUI], [
   AALSDK_PROG_GDB

   AC_ARG_VAR([GDBTUI], [GNU debugger Text User Interface])
   AS_IF([test "x${GDBTUI}" = x], [AC_PATH_PROG([GDBTUI], [gdbtui])])

   AS_IF([test "x${GDBTUI}" = x],
         [
          AM_COND_IF([COND_HAVE_GDB], dnl no gdbtui, but found gdb.
                     [
                      AS_IF([echo q | ${GDB} -tui >/dev/null 2>&1], dnl does gdb support -tui ?
                            [
                             AC_SUBST([GDBTUI], ["${GDB} -tui"]) dnl yes
                            ])
                     ])
         ])
   AM_CONDITIONAL([COND_HAVE_GDBTUI], [test "x${GDBTUI}" != x])
])

