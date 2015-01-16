dnl # VERIFY_OPTIONS
dnl # ---
AC_DEFUN([VERIFY_OPTIONS], [
   _conf_CPPFLAGS="${DEBUG_CPPFLAGS} ${ASSERT_CPPFLAGS}"
   _conf_CFLAGS="${CFLAGS}"
   _conf_CXXFLAGS="${CXXFLAGS}"
   _conf_LDFLAGS="${LDFLAGS}"
   _conf_LIBS="${LIBS}"

   AM_COND_IF([COND_WITH_GCOV],
              [ dnl requires -O0
                dnl prefer inline function when using gcov
               _conf_CPPFLAGS="${DEBUG_CPPFLAGS} ${ASSERT_CPPFLAGS}"
               _conf_CFLAGS="${GCOV_CFLAGS}"
               _conf_CXXFLAGS="${GCOV_CXXFLAGS}"
               _conf_LDFLAGS="${GCOV_LDFLAGS}"
               _conf_LIBS="${LIBS} ${GCOV_LIBS}"
               AM_COND_IF([COND_WITH_GPROF], [AC_MSG_ERROR([only one of gcov or gprof instrumentation may be selected.])])
              ])

   AM_COND_IF([COND_WITH_GPROF],
              [ dnl prefer inline functions when using gprof
               _conf_CPPFLAGS="${DEBUG_CPPFLAGS} ${ASSERT_CPPFLAGS}"
               _conf_CFLAGS="${GPROF_CFLAGS}"
               _conf_CXXFLAGS="${GPROF_CXXFLAGS}"
               _conf_LDFLAGS="${GPROF_LDFLAGS}"
              ])

dnl   AM_COND_IF([COND_WITH_VALGRIND],
dnl              [ dnl requires -O0
dnl               _conf_CFLAGS="${VALGRIND_CFLAGS}"
dnl               _conf_CXXFLAGS="${VALGRIND_CXXFLAGS}"
dnl               AM_COND_IF([COND_WITH_GPROF], [AC_MSG_ERROR([only one of valgrind or gprof instrumentation may be selected.])])
dnl              ])

dnl   AM_COND_IF([COND_HAVE_GDB],
dnl              [
dnl               _conf_CFLAGS=`echo ${_conf_CFLAGS} | sed -re "s,-g([[[:space:]]]|$),-ggdb${ac_cv_gdb_dbg_level}\1,"`
dnl               _conf_CXXFLAGS=`echo ${_conf_CXXFLAGS} | sed -re "s,-g([[[:space:]]]|$),-ggdb${ac_cv_gdb_dbg_level}\1,"`
dnl              ])

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
gcov Coverage Analysis is enabled. Running 'make && make check' will use gcov/lcov
to gather coverage analysis data for the instrumented applications."
])

AM_COND_IF([COND_WITH_GPROF], [
echo \
"
gprof Profiling is enabled. Running 'make && make check' will use gprof
to gather profiling data for the instrumented applications."
])

dnl AM_COND_IF([COND_WITH_VALGRIND], [
dnl echo \
dnl "
dnl valgrind analysis is enabled. Running 'make && make check'
dnl will use valgrind to analyze the instrumented applications."
dnl ])

echo \
"
${CC} ${CFLAGS}
${CXX} ${CXXFLAGS}
${LD} ${LDFLAGS} ${LIBS}
"
])
