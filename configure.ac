AC_PREREQ([2.63])
AC_INIT([Tali], [3.21.4], [https://bugzilla.gnome.org/],
        [tali], [https://wiki.gnome.org/Apps/Tali])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([1.11 no-dist-gzip dist-xz foreign])
AM_SILENT_RULES([yes])
AM_MAINTAINER_MODE
AC_CONFIG_HEADERS([config.h])

AM_PROG_CC_C_O

GLIB_GSETTINGS

APPSTREAM_XML

dnl ###########################################################################
dnl Dependencies
dnl ###########################################################################

GTK_REQUIRED=3.15.0
RSVG_REQUIRED=2.32.0

PKG_CHECK_MODULES(TALI, [
  gtk+-3.0 >= $GTK_REQUIRED
  librsvg-2.0 >= $RSVG_REQUIRED
])

AC_PATH_PROG([DESKTOP_FILE_VALIDATE], [desktop-file-validate], [/bin/true])

dnl ###########################################################################
dnl GResources
dnl ###########################################################################

AC_PATH_PROG(GLIB_COMPILE_RESOURCES, glib-compile-resources)

dnl ###########################################################################
dnl Internationalization
dnl ###########################################################################

IT_PROG_INTLTOOL([0.50])
AC_SUBST(GETTEXT_PACKAGE, tali)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [The gettext package name])

dnl ###########################################################################
dnl Documentation
dnl ###########################################################################

YELP_HELP_INIT

dnl ###########################################################################
dnl Files to generate
dnl ###########################################################################

AC_CONFIG_FILES([
Makefile
po/Makefile.in
data/Makefile
data/icons/Makefile
help/Makefile
src/Makefile
])
AC_OUTPUT
