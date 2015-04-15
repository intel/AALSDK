dnl INTEL CONFIDENTIAL - For Intel Internal Use Only

dnl # VERIFY_OPTIONS
dnl # ---
AC_DEFUN([VERIFY_OPTIONS], [
   _conf_CPPFLAGS="-DDBG_OSLTHREAD=1 ${DEBUG_CPPFLAGS} ${ASSERT_CPPFLAGS}"
   _conf_CFLAGS="${CFLAGS}"
   _conf_CXXFLAGS="${CXXFLAGS}"
   _conf_LDFLAGS="${LDFLAGS}"
   _conf_LIBS="${LIBS}"

   AM_COND_IF([COND_WITH_GCOV],
              [ dnl requires -O0 --coverage
               _conf_CPPFLAGS="${DEBUG_CPPFLAGS} ${ASSERT_CPPFLAGS}"
               _conf_CFLAGS="${GCOV_CFLAGS}"
               _conf_CXXFLAGS="${GCOV_CXXFLAGS}"
               _conf_LDFLAGS="${GCOV_LDFLAGS}"
               _conf_LIBS="${LIBS} ${GCOV_LIBS}"
              ])

   AM_COND_IF([COND_WITH_VALGRIND],
              [ dnl requires -O0 -fno-inline
               _conf_CFLAGS="${VALGRIND_CFLAGS}"
               _conf_CXXFLAGS="${VALGRIND_CXXFLAGS}"
               AM_COND_IF([COND_WITH_GCOV], [AC_MSG_ERROR([only one of gcov or valgrind instrumentation may be selected.])])
              ])

   AM_COND_IF([COND_WANT_GDB],
              [
               _conf_CFLAGS=`echo ${_conf_CFLAGS} | sed -re "s,-g([[[:space:]]]|$),-ggdb${ac_cv_gdb_dbg_level}\1,"`
               _conf_CXXFLAGS=`echo ${_conf_CXXFLAGS} | sed -re "s,-g([[[:space:]]]|$),-ggdb${ac_cv_gdb_dbg_level}\1,"`
              ])

   AC_SUBST([_conf_CPPFLAGS], ["${_conf_CPPFLAGS}"])
   AC_SUBST([_conf_CFLAGS],   ["${_conf_CFLAGS}"])
   AC_SUBST([_conf_CXXFLAGS], ["${_conf_CXXFLAGS}"])
   AC_SUBST([_conf_LDFLAGS],  ["${_conf_LDFLAGS}"])
   AC_SUBST([_conf_LIBS],     ["${_conf_LIBS}"])

   AC_CONFIG_COMMANDS_PRE([CPPFLAGS="${_conf_CPPFLAGS}"])
   AC_CONFIG_COMMANDS_PRE([CFLAGS="${_conf_CFLAGS}"])
   AC_CONFIG_COMMANDS_PRE([CXXFLAGS="${_conf_CXXFLAGS}"])
   AC_CONFIG_COMMANDS_PRE([LDFLAGS="${_conf_LDFLAGS}"])
   AC_CONFIG_COMMANDS_PRE([LIBS="${_conf_LIBS}"])
])

dnl # SHOW_OPTIONS
dnl # ---
AC_DEFUN([SHOW_OPTIONS], [

AM_COND_IF([COND_WITH_GCOV], [
echo \
"
code is being instrumented for gcov/lcov analysis."
])

AM_COND_IF([COND_WITH_VALGRIND], [
echo \
"
code is being instrumented for valgrind ${WITH_VALGRIND} analysis."
])

AM_COND_IF([COND_WANT_GDB], [
echo \
"
gdb was requested."
])

echo \
"
${CC} ${CFLAGS}
${CXX} ${CXXFLAGS}
${LD} ${LDFLAGS} ${LIBS}
"
])

