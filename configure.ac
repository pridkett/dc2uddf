dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(dc2uddf, 1.0)


AC_CANONICAL_SYSTEM
AM_INIT_AUTOMAKE()

AC_PROG_CC
AM_PROG_CC_C_O
PKG_CHECK_MODULES(XML, libxml-2.0 >= 2.4)
PKG_CHECK_MODULES(DIVECOMPUTER, libdivecomputer >= 0.3)
PKG_CHECK_MODULES(GLIB, glib-2.0 > 2.0)
PKG_CHECK_MODULES(CHECK, check >= 0.9.8)
AC_CONFIG_FILES(Makefile src/Makefile)
AC_OUTPUT

