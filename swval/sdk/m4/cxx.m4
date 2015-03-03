dnl INTEL CONFIDENTIAL - For Intel Internal Use Only

dnl # AALSDK_PROG_CXXFILT
dnl # ---
AC_DEFUN([AALSDK_PROG_CXXFILT], [
   AC_ARG_VAR([CXXFILT], [demangle C++ and Java symbols])
   AS_IF([test "x${CXXFILT}" = x], [AC_PATH_PROG([CXXFILT], [c++filt])])
])

