dnl configure.ac
dnl
dnl Autoconf configuration
dnl
dnl Use autoconf to process this into a configure script
dnl
dnl   (C) Copyright 2014 Fred Gleason <fredg@paravelsystems.com>
dnl
dnl      $Id: configure.ac,v 1.2 2014/02/18 20:16:45 cvs Exp $
dnl
dnl   This program is free software; you can redistribute it and/or modify
dnl   it under the terms of the GNU General Public License version 2 as
dnl   published by the Free Software Foundation.
dnl
dnl   This program is distributed in the hope that it will be useful,
dnl   but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl   GNU General Public License for more details.
dnl
dnl   You should have received a copy of the GNU General Public
dnl   License along with this program; if not, write to the Free Software
dnl   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
dnl

AC_INIT(glasscoder,`cat PACKAGE_VERSION`,Fred Gleason <fredg@paravelsystems.com>)
AM_INIT_AUTOMAKE(glasscoder,`cat PACKAGE_VERSION`)
AC_SUBST(RPM_RELEASE,1)
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

#
# Some Fully Resolved Install Paths
# (for the packaging system and friends)
#
if test ${prefix} = "NONE" ; then
  AC_SUBST(LOCAL_PREFIX,/usr/local)
else
  AC_SUBST(LOCAL_PREFIX,${prefix})
fi

#
# Basic Compiler Checks
#
AC_PROG_CXX
AC_PROG_LIBTOOL
AC_LANG(C++)

# to avoid rpath usage :
# http://wiki.debian.net/index.cgi?RpathIssue
case ${host} in
  *-pc-linux-gnu)
    AC_MSG_RESULT([Fixing libtool for -rpath problems.])
    sed < libtool > libtool-2 \
    's/^hardcode_libdir_flag_spec.*$/hardcode_libdir_flag_spec=" -D__LIBTOOL_IS_A_FOOL__ "/'
    mv libtool-2 libtool
    chmod 755 libtool
  ;;
esac

#
# Option Switches
#
AC_ARG_ENABLE(vorbis,[  --disable-vorbis        disable OggVorbis support],
	      [VORBIS_DISABLED=yes],[]) 
AC_ARG_ENABLE(twolame,[  --disable-twolame       disable MPEG Layer 2 encode support],
		      [TWOLAME_DISABLED=yes],[])
AC_ARG_ENABLE(lame,[  --disable-lame          disable MPEG Layer 3 support],
	      [LAME_DISABLED=yes],[]) 
AC_ARG_ENABLE(faac,[  --disable-faac          disable MPEG-2/4 AAC support],
	      [FAAC_DISABLED=yes],[]) 

#
# Check for Qt (Mandatory)
#
AC_CHECK_HEADER(QtCore/QtCore,[],[AC_MSG_ERROR([*** Qt4 not found ***])])
AC_CHECK_PROG(MOC_NAME,moc-qt4,[moc-qt4],[moc])
AC_SUBST(MOC,$MOC_NAME)

#
# Check for Jack Audio Connection Kit (Mandatory)
#
AC_CHECK_HEADER(jack/jack.h,[],[AC_MSG_ERROR([*** JACK not found ***])])

#
# Check for Secret Rabbit Code (Mandatory)
#
AC_CHECK_HEADER(samplerate.h,[],[AC_MSG_ERROR([*** libsamplerate not found ***])])

#
# Check for OggVorbis
#
AC_CHECK_HEADER(vorbis/vorbisfile.h,[VORBIS_FOUND=yes],[])
if test $VORBIS_FOUND = yes ; then
  if test -z $VORBIS_DISABLED ; then
    AC_DEFINE(HAVE_VORBIS)
    USING_VORBIS=yes
  fi
fi

#
# Check for TwoLame (Optional, required for MPEG Layer 2 support)
#
AC_CHECK_HEADER(twolame.h,[TWOLAME_FOUND=yes],[])
if test $TWOLAME_FOUND ; then
  if test -z $TWOLAME_DISABLED ; then
    AC_DEFINE(HAVE_TWOLAME)
    USING_TWOLAME=yes
  fi
fi

#
# Check for Lame (Optional, required for MPEG Layer 3 support)
#
AC_CHECK_HEADER(lame/lame.h,[LAME_FOUND=yes],[])
if test $LAME_FOUND ; then
  if test -z $LAME_DISABLED ; then
    AC_DEFINE(HAVE_LAME)
    USING_LAME=yes
  fi
fi

#
# Check for FAAC (Optional, required for MPEG-2/4 AAC support)
#
AC_CHECK_HEADER(faac.h,[FAAC_FOUND=yes],[])
if test $FAAC_FOUND ; then
  if test -z $FAAC_DISABLED ; then
    AC_DEFINE(HAVE_FAAC)
    USING_FAAC=yes
  fi
fi

#
# Determine the target architecture
#
AR_GCC_TARGET()
AC_SUBST(VENDOR,$ar_gcc_distro)
AC_SUBST(ARCH,$ar_gcc_arch)

#
# Determine Distro
#
AR_GET_DISTRO()
AC_SUBST(DISTRO,$ar_gcc_distro)
if test $ar_gcc_distro = suse ; then
  AC_SUBST(DIST_VER,"")
  AC_SUBST(DEBUG_RPM,"no")
else
  DISTRO_MAJOR_VER=`echo $ar_distro_version | awk 'split($1,fields,".") {print fields[[1]]}'`
  if test $DISTRO_MAJOR_VER -ge 6 ; then
    AC_SUBST(DIST_VER,".el$DISTRO_MAJOR_VER")
    AC_SUBST(DEBUG_RPM,"yes")
  else
    AC_SUBST(DIST_VER,"")
    AC_SUBST(DEBUG_RPM,"no")
  fi
fi

#
# Linux vs. OS X Setup
#
if test $ar_gcc_os = "darwin10" ; then
  AC_SUBST(SIRLIBS,"-lsamplerate -ljack")
  AC_SUBST(SIRFLAGS,"-framework QtCore -framework QtNetwork")
  AC_DEFINE(OSX)
else
  AC_SUBST(SIRLIBS,"-lsamplerate -ljack -lQtCore -lQtNetwork")
  AC_SUBST(SIRFLAGS,"")
  AC_DEFINE(LINUX)
fi

#
# Configure RPM Build
#
AC_CHECK_PROG(RPMBUILD_FOUND,rpmbuild,[yes],[])
if test -z $RPMBUILD_FOUND ; then
  AC_SUBST(RPMBUILD,rpm)
else
  AC_SUBST(RPMBUILD,rpmbuild)
fi
if test -d /usr/src/redhat ; then
  AC_SUBST(RPM_ROOT,/usr/src/redhat)
  AC_SUBST(VENDOR,redhat)
else
  AC_SUBST(RPM_ROOT,/usr/src/packages)
  AC_SUBST(VENDOR,suse)
fi 

AC_CONFIG_FILES([ \
    docs/Makefile \
    src/Makefile \
    glasscoder.spec \
    Makefile ])
AC_OUTPUT()

AC_MSG_NOTICE()
AC_MSG_NOTICE("|-----------------------------------------------------|")
AC_MSG_NOTICE("|       *** GLASSCODER CONFIGURATION SUMMARY ***      |")
AC_MSG_NOTICE("|-----------------------------------------------------|")
AC_MSG_NOTICE("| Encoding Formats:                                   |")
if test -z $USING_TWOLAME ; then
AC_MSG_NOTICE("|              MPEG-1 Layer 2 (via TwoLAME) ... No    |")
else
AC_MSG_NOTICE("|              MPEG-1 Layer 2 (via TwoLAME) ... Yes   |")
fi
if test -z $USING_LAME ; then
AC_MSG_NOTICE("|                 MPEG-1 Layer 3 (via LAME) ... No    |")
else
AC_MSG_NOTICE("|                 MPEG-1 Layer 3 (via LAME) ... Yes   |")
fi
if test -z $USING_FAAC ; then
AC_MSG_NOTICE("|                     MPEG-2 AAC (via FAAC) ... No    |")
else
AC_MSG_NOTICE("|                     MPEG-2 AAC (via FAAC) ... Yes   |")
fi
if test -z $USING_VORBIS ; then
AC_MSG_NOTICE("|   OggVorbis (via libogg and libvorbisenc) ... No    |")
else
AC_MSG_NOTICE("|   OggVorbis (via libogg and libvorbisenc) ... Yes   |")
fi
AC_MSG_NOTICE("|-----------------------------------------------------|")
AC_MSG_NOTICE()
AC_MSG_NOTICE()
AC_MSG_NOTICE(Now enter 'make' to build the software.)
AC_MSG_NOTICE()
