dnl INTEL CONFIDENTIAL - For Intel Internal Use Only

dnl # AALSDK_LOCAL_GTEST(REL-PATH, VERSION)
dnl # ---
AC_DEFUN([AALSDK_LOCAL_GTEST], [
   AC_MSG_CHECKING([local gtest tarball])
   AS_IF([test -f "${srcdir}/$1/gtest-$2.tar.gz"],
         [
          AC_MSG_RESULT([${srcdir}/$1/gtest-$2.tar.gz])
          _gtest_defines=
          AC_CHECK_HEADERS([tr1/tuple], [_gtest_defines='-DGTEST_HAS_TR1_TUPLE=1'],
                           [AC_MSG_WARN([tr1/tuple not found. Some gtest features will not be available.])])
          AS_IF([test -d "${srcdir}/$1/gtest-$2"], [],
                [tar -C "${srcdir}/$1" -zxf "${srcdir}/$1/gtest-$2.tar.gz"])
          AC_SUBST([WITH_GTEST],           [yes])
          AC_DEFINE([WITH_LOCAL_GTEST],    [1], [1 if local gtest tarball found])
          AM_CONDITIONAL([AAL_COND_GTEST], [true])
          AC_SUBST([GTEST_CPPFLAGS],       ["${_gtest_defines} -I`cd ${srcdir} && pwd`/$1/gtest-$2/include -I`cd ${srcdir} && pwd`/$1/gtest-$2"])
          AC_SUBST([GTEST_CFLAGS],         ['-g -O2'])
          AC_SUBST([GTEST_CXXFLAGS],       ['-g -O2'])
          AC_SUBST([GTEST_LDFLAGS],        ['-avoid-version'])
          AC_SUBST([GTEST_SRCDIR],         ["`cd ${srcdir} && pwd`/$1/gtest-$2/src"])
         ],
         [
          AC_MSG_RESULT([not found])
          AC_SUBST([WITH_GTEST],           [no])
          AM_CONDITIONAL([AAL_COND_GTEST], [false])
          AC_SUBST([GTEST_CPPFLAGS],       [])
          AC_SUBST([GTEST_CFLAGS],         [])
          AC_SUBST([GTEST_CXXFLAGS],       [])
          AC_SUBST([GTEST_LDFLAGS],        [])
          AC_SUBST([GTEST_SRCDIR],         [])
         ])
]) dnl # AALSDK_LOCAL_GTEST

