# Copyright 1999-2010 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# Author: Guillaume Bouchard <guillaume@apinc.org>


EAPI=3

DESCRIPTION="Client for patacrep songbooks
The songbook-client is an interface to build
 customized PDF songbooks with lyrics, guitar
  chords and lilypond sheets from patacrep songbook."
HOMEPAGE="http://www.patacrep.com"
EGIT_REPO_URI="http://github.com/crep4ever/songbook-client.git"

inherit git autotools eutils

LICENSE="GPL-2+"
SLOT="0"
KEYWORDS="amd64 x86"
IUSE="git lilypond"

DEPEND="x11-libs/qt-gui
       app-text/texlive[linguas_fr,extra]
       dev-lang/python
       dev-util/cmake
       x11-libs/qt-sql[sqlite]
       
       git? ( dev-vcs/git )
       lilypond? ( media-sound/lilypond )
       "

RDEPEND="${DEPEND}"

src_install () {
   emake DESTDIR="$D" install || die
}


