#!/bin/bash

# You may override these from the environment
: ${CMAKE_COMMAND:=cmake}

###############################################################################

if [ -z "$($CMAKE_COMMAND) --version" ] ; then
	echo "You have to install CMake" >&2
	exit 1
fi

if [ -z "$RTTR_SRCDIR" ] ; then
	RTTR_SRCDIR="$(dirname "$0")/.."
fi

###############################################################################

mecho()
{
	COLOR=$1
	shift
	$CMAKE_COMMAND -E cmake_echo_color --bold $COLOR "$*"
}

###############################################################################

if [ ! -f cleanup.sh ] && [ -f "${RTTR_SRCDIR}/build/cleanup.sh" ] ; then
	mecho --blue "Creating symlink 'cleanup.sh' ..."
	ln -vs $RTTR_SRCDIR/build/cleanup.sh cleanup.sh
fi

if [ ! -f cmake.sh ] && [ -f "${RTTR_SRCDIR}/build/cmake.sh" ] ; then
	mecho --blue "Creating symlink 'cmake.sh' ..."
	ln -vs $RTTR_SRCDIR/build/cmake.sh cmake.sh
fi

###############################################################################

RTTR_INSTALL_PREFIX=/usr/local
RTTR_BINDIR=
RTTR_DATADIR=
RTTR_LIBDIR=
RTTR_TOOL_CHAIN=
RTTR_NOARCH=
GENERATOR=
PARAMS=""
as_cr_letters='abcdefghijklmnopqrstuvwxyz'
as_cr_LETTERS='ABCDEFGHIJKLMNOPQRSTUVWXYZ'
as_cr_Letters=$as_cr_letters$as_cr_LETTERS
as_cr_digits='0123456789'
as_cr_alnum=$as_cr_Letters$as_cr_digits

while test $# != 0 ; do
	case $1 in
		--*=*)
			ac_option=`expr "X$1" : 'X\([^=]*\)='`
			ac_optarg=`expr "X$1" : 'X[^=]*=\(.*\)'`
			ac_shift=:
		;;
		-*=*)
			ac_option=`expr "X$1" : 'X\([^=]*\)='`
			ac_optarg=`expr "X$1" : 'X[^=]*=\(.*\)'`
			ac_shift=:
		;;
		*)
			ac_option=$1
			ac_optarg=yes
			ac_shift=shift
		;;
	esac

	case $ac_option in
		-prefix | --prefix)
			$ac_shift
			RTTR_INSTALL_PREFIX=$ac_optarg
		;;
		-RTTR_BINDIR | --RTTR_BINDIR)
			$ac_shift
			RTTR_BINDIR=$ac_optarg
		;;
		-RTTR_DATADIR | --RTTR_DATADIR)
			$ac_shift
			RTTR_DATADIR=$ac_optarg
		;;
		-RTTR_LIBDIR | --RTTR_LIBDIR)
			$ac_shift
			RTTR_LIBDIR=$ac_optarg
		;;
		-arch | --arch | -target | --target | -toolchain | --toolchain)
			$ac_shift
			RTTR_TOOL_CHAIN=$ac_optarg
		;;
		-no-arch | --no-arch)
			$ac_shift
			RTTR_NOARCH="$RTTR_NOARCH $ac_optarg"
		;;
		-generator | --generator)
			#$ac_shift
			GENERATOR="$ac_optarg"
		;;
		-enable-* | --enable-*)
			ac_feature=`expr "x$ac_option" : 'x-*enable-\([^=]*\)'`
			# Reject names that are not valid shell variable names.
			expr "x$ac_feature" : ".*[^-._$as_cr_alnum]" >/dev/null &&
			{
				echo "error: invalid feature name: $ac_feature" >&2
				{ (exit 1); exit 1; };
			}
			ac_feature=`echo $ac_feature | sed 's/[-.]/_/g'`
			if [ -z "$ac_optarg" ] ; then
				ac_optarg="yes"
			fi
			eval enable_$ac_feature=\$ac_optarg
		;;
		-disable-* | --disable-*)
		        ac_feature=`expr "x$ac_option" : 'x-*disable-\([^=]*\)'`
						# Reject names that are not valid shell variable names.
						expr "x$ac_feature" : ".*[^-._$as_cr_alnum]" >/dev/null &&
						{
								echo "error: invalid feature name: $ac_feature" >&2
								{ (exit 1); exit 1; };
						}
						ac_feature=`echo $ac_feature | sed 's/[-.]/_/g'`
						if [ -z "$ac_optarg" ] ; then
								ac_optarg="yes"
						fi
						eval disable_$ac_feature=\$ac_optarg
		;;
		-D*)
			PARAMS="$PARAMS $ac_option=$ac_optarg"
		;;
		*)
			echo "Unknown option: $ac_option"
			exit 1
		;;
	esac

	shift
done

###############################################################################

echo "Setting Install-Prefix to \"$RTTR_INSTALL_PREFIX\""
PARAMS="$PARAMS -DCMAKE_INSTALL_PREFIX=$RTTR_INSTALL_PREFIX"

case "$RTTR_TOOL_CHAIN" in
	*local*)
		RTTR_TOOL_CHAIN=
		;;
esac

if [ ! -z "$RTTR_TOOL_CHAIN" ] ; then
    echo "Using toolchain \"$RTTR_TOOL_CHAIN\""
    PARAMS="$PARAMS -DCMAKE_TOOLCHAIN_FILE=${RTTR_SRCDIR}/cmake/toolchains/$RTTR_TOOL_CHAIN.cmake"	
fi

if [ ! -z "$RTTR_BINDIR" ]; then
    echo "Setting Binary Dir to \"$RTTR_BINDIR\""
    PARAMS="$PARAMS -DRTTR_BINDIR=$RTTR_BINDIR"
fi

if [ ! -z "$RTTR_DATADIR" ]; then
    echo "Setting Data Dir to \"$RTTR_DATADIR\""
    PARAMS="$PARAMS -DRTTR_DATADIR=$RTTR_DATADIR"
fi

if [ ! -z "$RTTR_LIBDIR" ]; then
    echo "Setting Library Dir to \"$RTTR_LIBDIR\""
    PARAMS="$PARAMS -DRTTR_LIBDIR=$RTTR_LIBDIR"
fi

if [ ! -z "$RTTR_NOARCH" ] ; then
    echo "Disabling build of \"$RTTR_NOARCH\""
fi
if [ ! -z "$disable_arch" ] ; then
	RTTR_NOARCH="$disable_arch $RTTR_NOARCH"
fi
for I in $RTTR_NOARCH ; do
	if [ ! -z "$I" ] ; then
		PARAMS="$PARAMS -DRTTR_NO$I=$I"
	fi
done

if [[ $PARAMS != *"-DCMAKE_BUILD_TYPE="* ]]; then
    if [[ "$enable_debug" =~ ^[Yy][Ee][Ss]$ ]]; then
		mecho --magenta "Activating debug build"
		PARAMS="$PARAMS -DCMAKE_BUILD_TYPE=Debug"
    elif [[ "$enable_reldeb" =~ ^[Yy][Ee][Ss]$ ]]; then
		mecho --magenta "Activating release build with debug information"
		PARAMS="$PARAMS -DCMAKE_BUILD_TYPE=RelWithDebInfo"
	else
		mecho --magenta "Activating release build"
		PARAMS="$PARAMS -DCMAKE_BUILD_TYPE=Release"
	fi
fi

if [[ $PARAMS != *"-DCMAKE_VERBOSE_MAKEFILE="* ]]; then
    if [[ "$enable_verbose" =~ ^[Yy][Ee][Ss]$ ]]; then
		mecho --magenta "Activating verbose build"
		PARAMS="$PARAMS -DCMAKE_VERBOSE_MAKEFILE=On"
	fi
fi

###############################################################################

if [ -z "$GENERATOR" ] && [ "$(uname -s)" = "Darwin" ] ; then
	# Check for existance of xcodebuild
	command -v xcodebuild >/dev/null 2>&1 && GENERATOR="Xcode"
fi

if [ -z "$GENERATOR" ] ; then
	GENERATOR="Unix Makefiles"
fi

echo "Generating files for \"$GENERATOR\""

mecho --blue "Running \"cmake -G '$GENERATOR' ${PARAMS} '${RTTR_SRCDIR}'\""
$CMAKE_COMMAND -G "$GENERATOR" $PARAMS "${RTTR_SRCDIR}"

if [ $? != 0 ] ; then
	mecho --red "An error occured - please check above!"
	exit 1
fi

if [ "$GENERATOR" = "Xcode" ]; then
	MAKE="xcodebuild"
else
	MAKE="make"
fi

mecho --blue "Now type \"$MAKE\" to build project"

exit 0

###############################################################################
