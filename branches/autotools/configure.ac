AC_INIT(heekscad, 0.6.0)

AC_PREREQ([2.64])
AM_INIT_AUTOMAKE(AC_PACKAGE_NAME, AC_PACKAGE_VERSION)
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_AWK
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_RANLIB

AC_LANG(C++)

#OCC_INCLUDE=/user/include/opencascade
#CPPFLAGS="$CPPFLAGS -I${OCC_INCLUDE}"
#CXXFLAGS="$CXXFLAGS -I${OCC_INCLUDE}"

AC_MSG_CHECKING(for extraneous occ header directories)
	  found_occ_dir=""
	  for occ_dir in /usr/include/opencascade /usr/local/include/opencascade $prefix/include
	  do

	  echo checking dir $occ_dir

	  if test -d $occ_dir ; then

	     echo FOUND OCC dir $occ_dir
	     OCC_INCLUDE=$occ_dir
	     CPPFLAGS="$CPPFLAGS -I${OCC_INCLUDE}"
	     CXXFLAGS="$CXXFLAGS -I${OCC_INCLUDE}"

	     AC_CHECK_HEADER([Standard.hxx],[
	      	echo found Standard.hxx
		AC_DEFINE(HAVE_OPENCASCADE_H, 1,[Define to 1 if you have <Standard.hxx.h>.]) ],[
		AC_MSG_ERROR([echo did nor find Standard.hxx])],)

	     found_occ_dir=" $found_occ_dir -I$occ_dir"

	    else
		echo did not find anything in $occ_dir
	    fi
	  done
	  if test "$found_occ_dir" != "" ; then
	    AC_MSG_RESULT(found the OCC DIR $found_occ_dir and added to CFLAGS)
	  else
	    AC_MSG_ERROR(opencascade not found)
	  fi


dnl AC_CHECK_HEADER([opencascade/Standard.hxx],[
dnl 	echo found Standard.hxx
dnl 	AC_DEFINE(HAVE_OPENCASCADE_H, 1,[Define to 1 if you have <Standard.hxx.h>.]) ],[
dnl 	AC_MSG_WARN([echo did nor find Standard.hxx])],)
dnl OCC_INCLUDE=/usr/local/include/opencascade
dnl CPPFLAGS="$CPPFLAGS -I${OCC_INCLUDE}"
dnl CXXFLAGS="$CXXFLAGS -I${OCC_INCLUDE}"
dnl AC_CHECK_HEADER([opencascade/Standard.hxx],[
dnl 	echo found Standard.hxx
dnl 	AC_DEFINE(HAVE_OPENCASCADE_H, 1,[Define to 1 if you have <Standard.hxx.h>.]) ],[
dnl 	AC_MSG_ERROR([echo did nor find Standard.hxx])],)


dnl AC_ARG_WITH(occ-prefix, 
dnl [ --with-occ-prefix=DIR   Prefix where open cascade is installed],[
dnl   OCC_PREFIX=$withval;
dnl   OCC_INCLUDE=${OCC_PREFIX}/include/opencascade
dnl   CPPFLAGS="$CPPFLAGS -I${OCC_INCLUDE}"
dnl   CXXFLAGS="$CXXFLAGS -I${OCC_INCLUDE}"
dnl   echo "using "$OCC_PREFIX" for occ-prefix";
dnl   echo "using "$CPPFLAGS" for CPPFLAGS";
dnl   AC_LANG_PUSH(C++)
dnl #
dnl   AC_CHECK_HEADER([Standard.hxx],[echo found Standard.hxx],[echo did nor find Standard.hxx],)
dnl  # AC_CHECK_HEADER([Standard_PrimitiveTypes.hxx],[echo found Standard_PrimitiveTypes.hxx],[echo did nor find Standard_PrimitiveTypes.hxx],)
dnl #  AC_CHECK_HEADER([BRepAdaptor_Curve.hxx],[echo found BRepAdaptor_Curve.hxx],[echo did nor find BRepAdaptor_Curve.hxx],)
dnl #  AC_CHECK_HEADER([opencascade/BRepAdaptor_Curve.hxx])
dnl #  AC_CHECK_HEADER([/usr/include/opencascade/BRepAdaptor_Curve.hxx])
dnl ],[
dnl   OCC_PREFIX=/usr/;
dnl   OCC_INCLUDE=${OCC_PREFIX}/include/opencascade
dnl ]
dnl )

m4_include(m4/wxwin.m4)	

AM_OPTIONS_WXCONFIG
reqwx=2.4.0
AM_PATH_WXCONFIG($reqwx, wxWin=1)
if test "$wxWin" != 1; then
	AC_MSG_ERROR([
		wxWidgets must be installed on your system.
 
		Please check that wx-config is in path, the directory
		where wxWidgets libraries are installed (returned by
		'wx-config --libs' or 'wx-config --static --libs' command)
		is in LD_LIBRARY_PATH or equivalent variable and
		wxWidgets version is $reqwx or above.
		])
fi
 
CPPFLAGS="$CPPFLAGS $WX_CPPFLAGS"
CXXFLAGS="$CXXFLAGS $WX_CXXFLAGS_ONLY"
CFLAGS="$CFLAGS $WX_CFLAGS_ONLY"
LIBS="$LIBS $WX_LIBS"


# Checks for libraries.
# FIXME: Replace `main' with a function in `-lFWOSPlugin':
AC_CHECK_LIB([FWOSPlugin], [_ZNK18Standard_Transient4ThisEv])
# FIXME: Replace `main' with a function in `-lTKAdvTools':
AC_CHECK_LIB([TKAdvTools], [_ZNK18Materials_Material4DumpERSo])
# FIXME: Replace `main' with a function in `-lTKBO':
AC_CHECK_LIB([TKBO], [_ZN27TColStd_IndexedMapOfInteger3AddERKi])
# FIXME: Replace `main' with a function in `-lTKBRep':
AC_CHECK_LIB([TKBRep], [main])
# FIXME: Replace `main' with a function in `-lTKBool':
AC_CHECK_LIB([TKBool], [main])
# FIXME: Replace `main' with a function in `-lTKCAF':
AC_CHECK_LIB([TKCAF], [main])
# FIXME: Replace `main' with a function in `-lTKCDF':
AC_CHECK_LIB([TKCDF], [main])
# FIXME: Replace `main' with a function in `-lTKFeat':
AC_CHECK_LIB([TKFeat], [main])
# FIXME: Replace `main' with a function in `-lTKFillet':
AC_CHECK_LIB([TKFillet], [main])
# FIXME: Replace `main' with a function in `-lTKG2d':
AC_CHECK_LIB([TKG2d], [main])
# FIXME: Replace `main' with a function in `-lTKG3d':
AC_CHECK_LIB([TKG3d], [main])
# FIXME: Replace `main' with a function in `-lTKGeomAlgo':
AC_CHECK_LIB([TKGeomAlgo], [main])
# FIXME: Replace `main' with a function in `-lTKGeomBase':
AC_CHECK_LIB([TKGeomBase], [main])
# FIXME: Replace `main' with a function in `-lTKHLR':
AC_CHECK_LIB([TKHLR], [main])
# FIXME: Replace `main' with a function in `-lTKIGES':
AC_CHECK_LIB([TKIGES], [main])
# FIXME: Replace `main' with a function in `-lTKLCAF':
AC_CHECK_LIB([TKLCAF], [main])
# FIXME: Replace `main' with a function in `-lTKMath':
AC_CHECK_LIB([TKMath], [main])
# FIXME: Replace `main' with a function in `-lTKMesh':
AC_CHECK_LIB([TKMesh], [main])
# FIXME: Replace `main' with a function in `-lTKOffset':
AC_CHECK_LIB([TKOffset], [main])
# FIXME: Replace `main' with a function in `-lTKPLCAF':
AC_CHECK_LIB([TKPLCAF], [main])
# FIXME: Replace `main' with a function in `-lTKPShape':
AC_CHECK_LIB([TKPShape], [main])
# FIXME: Replace `main' with a function in `-lTKPrim':
AC_CHECK_LIB([TKPrim], [main])
# FIXME: Replace `main' with a function in `-lTKSTEP':
AC_CHECK_LIB([TKSTEP], [main])
# FIXME: Replace `main' with a function in `-lTKSTEP209':
AC_CHECK_LIB([TKSTEP209], [main])
# FIXME: Replace `main' with a function in `-lTKSTEPAttr':
AC_CHECK_LIB([TKSTEPAttr], [main])
# FIXME: Replace `main' with a function in `-lTKSTEPBase':
AC_CHECK_LIB([TKSTEPBase], [main])
# FIXME: Replace `main' with a function in `-lTKSTL':
AC_CHECK_LIB([TKSTL], [main])
# FIXME: Replace `main' with a function in `-lTKService':
AC_CHECK_LIB([TKService], [main])
# FIXME: Replace `main' with a function in `-lTKShHealing':
AC_CHECK_LIB([TKShHealing], [main])
# FIXME: Replace `main' with a function in `-lTKShapeSchema':
AC_CHECK_LIB([TKShapeSchema], [main])
# FIXME: Replace `main' with a function in `-lTKTopAlgo':
AC_CHECK_LIB([TKTopAlgo], [main])
# FIXME: Replace `main' with a function in `-lTKV2d':
AC_CHECK_LIB([TKV2d], [main])
# FIXME: Replace `main' with a function in `-lTKV3d':
AC_CHECK_LIB([TKV3d], [main])
# FIXME: Replace `main' with a function in `-lTKVRML':
AC_CHECK_LIB([TKVRML], [main])
# FIXME: Replace `main' with a function in `-lTKXCAF':
AC_CHECK_LIB([TKXCAF], [main])
# FIXME: Replace `main' with a function in `-lTKXCAFSchema':
AC_CHECK_LIB([TKXCAFSchema], [main])
# FIXME: Replace `main' with a function in `-lTKXDESTEP':
AC_CHECK_LIB([TKXDESTEP], [main])
# FIXME: Replace `main' with a function in `-lTKXSBase':
AC_CHECK_LIB([TKXSBase], [main])
# FIXME: Replace `main' with a function in `-lTKernel':
AC_CHECK_LIB([TKernel], [main])

# Checks for header files.
AC_CHECK_HEADERS([locale.h stddef.h stdlib.h string.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T
AC_CHECK_TYPES([ptrdiff_t])

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_STRTOD
AC_CHECK_FUNCS([floor memmove memset pow setlocale sqrt strchr strtoul])


dnl **************************
dnl check for open cascade
dnl **************************
# taken from freeengineer/freedraft ftp://ftp.freeengineer.org/pub/freedraft/snapshots/freedraft20020309.tar.gz



AC_CONFIG_FILES([Makefile
                 src/Makefile
])
#                 translations/Makefile
#                 unittest/Makefile

AC_OUTPUT
