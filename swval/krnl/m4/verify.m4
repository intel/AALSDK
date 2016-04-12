dnl INTEL CONFIDENTIAL - For Intel Internal Use Only

dnl # VERIFY_OPTIONS
dnl # ---
AC_DEFUN([VERIFY_OPTIONS], [
   _conf_CPPFLAGS=" ${DEBUG_CPPFLAGS} ${ASSERT_CPPFLAGS}"
   _conf_CFLAGS="${CFLAGS}"
   _conf_CXXFLAGS="${CXXFLAGS}"
   _conf_LDFLAGS="${LDFLAGS}"
   _conf_LIBS="${LIBS}"



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

echo \
"
${CC} ${CFLAGS}
"
])

