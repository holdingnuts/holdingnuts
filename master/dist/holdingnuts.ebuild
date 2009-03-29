# Copyright 1999-2009 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: $

EAPI="2"
inherit eutils games cmake-utils

DESCRIPTION="An open source poker client and server"
HOMEPAGE="http://www.holdingnuts.net/"
SRC_URI="http://downloads.sourceforge.net/${PN}/${P}.tar.bz2"

LICENSE="GPL-3"
SLOT="0"
KEYWORDS="~amd64 ~x86"
IUSE="alsa debug dedicated"

RDEPEND="!dedicated? (
	>=x11-libs/qt-core-4.4.1
	>=x11-libs/qt-gui-4.4.1
	alsa? (
		>=media-libs/libsdl-1.2.10[alsa]
	)
)"

DEPEND="${RDEPEND}
	>=dev-util/cmake-2.6.3"
# older CMake-2.6 versions should not be used because of a bug in
# CMake's Qt module: moc-qt4 isn't provided with CPP definitions

src_configure() {
	local mycmakeargs="$(cmake-utils_use_enable alsa AUDIO)
		$(cmake-utils_use_enable !dedicated CLIENT)
		$(cmake-utils_use_enable debug DEBUG)"
	
	mycmakeargs="${mycmakeargs}
		-DCMAKE_INSTALL_PREFIX=${GAMES_PREFIX}
		-DCMAKE_DATA_PATH=${GAMES_DATADIR}"
	
	cmake-utils_src_configure
}

src_install() {
	cmake-utils_src_install
	
	if ! use dedicated ; then
		domenu ${PN}.desktop
		doicon ${PN}.png
	fi
	
	dodoc docs/protocol_spec.txt
	
	prepgamesdirs
}
