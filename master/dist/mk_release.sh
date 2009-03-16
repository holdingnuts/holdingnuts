#!/bin/bash
# release script

if [[ $# != 2 && $# != 4 ]] ; then
	echo "mk_release.sh <trunk-dir> <version>" >&2
	echo "mk_release.sh <trunk-dir> <build-dir> <platform> <version>" >&2
	exit 0
elif [[ $# == 2 ]] ; then
	src_release=1
fi


TRUNK=$1

if [[ ${src_release} ]] ; then
	RELVER=$2
else
	BUILD=$2
	PLATFORM=$3
	RELVER=$4
fi

if [[ ! -d ${TRUNK} ]] ; then
	echo "Error: Trunk dir does not exist." >&2
	exit 1
fi

if [[ ! ${src_release} ]] ; then
	if [[ ! -d ${BUILD} ]] ; then
		echo "Error: Build dir does not exist." >&2
		exit 1
	fi

	if [[ ${PLATFORM} != linux_x86 && ${PLATFORM} != linux_x86_64 && ${PLATFORM} != win32 ]] ; then
		echo "Error: Unsupported platform: ${PLATFORM}" >&2
		exit 1
	fi

	case ${PLATFORM} in
	win* )
		convert_text=1
		is_windows=1
		;;
	esac
	
	PKGSTR="holdingnuts-${RELVER}-${PLATFORM}"
else
	PKGSTR="holdingnuts-${RELVER}-src"
fi



TMPDIR=$PKGSTR

echo "--- mk_release.sh running for ${PKGSTR} ---"

if [[ ! ${src_release} ]] ; then
	echo
	echo "IMPORTANT:"
	echo "Configure and build the sources with"
	echo "  cmake -DCMAKE_BUILD_TYPE=Release -DUSE_SVNREV=On"
	echo "and build the translations with"
	echo "  make translations"
	echo "If this wasn't done, the release package will be incomplete!"
fi

echo
echo "Checking for unmodified trunk..."
SVNREV=$(svnversion ${TRUNK})
echo "SVN revision: ${SVNREV}"
svn st --no-ignore ${TRUNK}

echo
echo "--- Verify and press ENTER to continue ---"
read

if [[ -d ${TMPDIR} ]] ; then
	echo "Deleting old temporary working directory..."
	rm -rf ${TMPDIR}
fi


echo "Creating temporary working directory ${TMPDIR}..."
mkdir ${TMPDIR}


if [[ ! ${src_release} ]] ; then
	if [[ ${convert_text} ]] ; then
		echo "Copying and converting license file..."
		awk '{ sub(/$/,"\r"); print }' ${TRUNK}/LICENSE > ${TMPDIR}/LICENSE.txt    # convert CRLF to LF
	else
		echo "Copying license file..."
		cp ${TRUNK}/LICENSE ${TMPDIR}/LICENSE
	fi


	echo "Copying data-dir..."
	cp -r -t ${TMPDIR} ${TRUNK}/data


	echo "Copying binaries..."
	if [[ $is_windows ]] ; then
		cp -t ${TMPDIR} ${BUILD}/src/client/holdingnuts.exe
		cp -t ${TMPDIR} ${BUILD}/src/server/holdingnuts-server.exe
	else
		cp -t ${TMPDIR} ${BUILD}/src/client/holdingnuts
		cp -t ${TMPDIR} ${BUILD}/src/server/holdingnuts-server
		
		# wrapper script
		cp -t ${TMPDIR} files/holdingnuts.sh
	fi

	echo "Copying translations..."
	cp -r -t  ${TMPDIR}/data/i18n ${BUILD}/src/client/*.qm

	if [[ ! $is_windows ]] ; then
		echo "Creating empty lib directory"
		mkdir ${TMPDIR}/lib
	fi

	echo "Copying shortcuts..."
	if [[ $is_windows ]] ; then
		cp -t ${TMPDIR} files/HoldingNuts.net.url
		cp -t ${TMPDIR} files/config_directory.lnk
	else
		cp -t ${TMPDIR} files/HoldingNuts.net.desktop
	fi

	echo "Copying logo..."
	cp ${TRUNK}/src/client/res/hn_logo.png ${TMPDIR}/holdingnuts.png
	
	echo "!!! Do not forget to"
	echo " * copy the qt_translations (e.g. qt_de.qm) into ${TMPDIR}/data/i18n as well"
	echo " * copy the libaries (windows: *.dll into ${TMPDIR} ; linux: *.so into ${TMPDIR}/lib"
else
	echo "Copying trunk..."
	cp -r -t ${TMPDIR} ${TRUNK}/*
	
	echo "Touching svn revision info file..."
	touch ${TMPDIR}/svn_r${SVNREV}
fi


echo "Removing svn entries..."
rm -rf $(find ${TMPDIR} -type d -name '.svn')

echo "Finished."
