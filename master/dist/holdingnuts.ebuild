# Copyright 1999-2009 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: $

EAPI=2
inherit eutils games

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
				>=media-libs/libsdl-1.2.10
			)
		)"

DEPEND="${RDEPEND}
	>=dev-util/cmake-2.6.3"
# older CMake-2.6 versions should not be used because of a bug in
# CMake's Qt module: moc-qt4 isn't provided with CPP definitions

src_configure() {
	local mycmakeargs
	use alsa || mycmakeargs="${mycmakeargs} -DENABLE_AUDIO=Off"
	use debug && \
		mycmakeargs="${mycmakeargs} -DCMAKE_BUILD_TYPE=Debug" || \
		mycmakeargs="${mycmakeargs} -DCMAKE_BUILD_TYPE=None"
	use dedicated && mycmakeargs="${mycmakeargs} -DENABLE_CLIENT=Off"
	#use test && mycmakeargs="${mycmakeargs} -DENABLE_TEST=On"
	cmake ${mycmakeargs} \
		-DCMAKE_C_COMPILER=$(type -P $(tc-getCC)) \
		-DCMAKE_C_FLAGS="${CFLAGS}" \
		-DCMAKE_CXX_COMPILER=$(type -P $(tc-getCXX)) \
		-DCMAKE_CXX_FLAGS="${CXXFLAGS}" \
		-DCMAKE_INSTALL_PREFIX=${GAMES_PREFIX} \
		-DDATA_DIR="${GAMES_DATADIR}"/${PN}/data \
		|| die "cmake failed"
}

src_compile() {
	emake || die "emake failed"
	if ! use dedicated ; then
		emake translations || die "emake failed"
	fi
}

src_install() {
	dogamesbin src/server/${PN}-server || die "dogamesbin failed"
	if ! use dedicated ; then
		dogamesbin src/client/${PN} || die "dogamesbin failed"
		insinto "${GAMES_DATADIR}/${PN}"
		doins -r data || die "doins failed"
		insinto "${GAMES_DATADIR}/${PN}/data/i18n"
		doins src/client/*.qm || die "doins failed"
		domenu ${PN}.desktop
		doicon ${PN}.png
	fi
	dodoc docs/protocol_spec.txt
	prepgamesdirs
}
