MAJOR_VERSION=1
MINOR_VERSION=0
PATCH_VERSION=9
VERSION=1.0.9
DEFINES += MAJOR_VERSION=$$MAJOR_VERSION MINOR_VERSION=$$MINOR_VERSION PATCH_VERSION=$$PATCH_VERSION
DEFINES += VERSION="$$VERSION"

#
#  Use "qmake -o Makefile loganalyz.pro" for release code, and
#  use "qmake -o Makefile loganalyz.pro debug=1" for debug code.
#
#  Adapt these variables to your system

# DEFINES += PRINT_CALLS


win32 {
	QMAKE_CXXFLAGS+=-mno-cygwin
	QMAKE_LFLAGS+=-mno-cygwin
	}


isEmpty(PREFIX){
    PREFIX = /usr/local
}
SHAREDIR = $${PREFIX}/share
DOCDIR = $${SHAREDIR}/doc/packages/loganalyz
BINDIR = $${PREFIX}/bin
TRANSLATIONS = parktronic_ru.ts

###############################################################################

FORMS +=

################################################################################

win32 {
	INCLUDEPATH += $$quote(C:/cygwin/usr/include/w32api/directx)
#	INCLUDEPATH += $$quote(C:/soft/novorado/windows/vlc-1.0.3/sdk/include)
	INCLUDEPATH += $$quote(C:/soft/novorado/windows/vlc-1.1.4/sdk/include)
	INCLUDEPATH += $$quote(C:/soft/novorado/windows/SDL-1.2.13/include)
	INCLUDEPATH += $$quote(C:/Tcl/include)
#	LIBPATH += $$quote(C:/soft/novorado/windows/vlc-1.0.3/sdk/lib)
	LIBPATH += $$quote(C:/soft/novorado/windows/vlc-1.1.4/sdk/lib)
	LIBPATH += $$quote(C:/soft/novorado/windows/SDL-1.2.13/lib)
	LIBPATH += $$quote(C:/Tcl/lib)
	LIBPATH += $(NOVORADO_DISTRIBS)/festival/src/lib
	LIBPATH += $(NOVORADO_DISTRIBS)/speech_tools/lib
	LIBPATH += $(INSTALL_LIB)
#	LIBPATH += "C:/cygwin/lib" -lcygwin -ltermcap
	DEFINES += NVSPEECH_PATH=$$quote(\\\"c:/soft/novorado/linux/nvspeech\\\")
	DEFINES += SYSTEM_IS_WIN32=1 MINGW=1
	# DirectX
	LIBS +=  -lstrmiids -lole32 -loleaut32 $$quote(C:/cygwin/lib/w32api/libuuid.a)
	LIBS += -ltcl86 -lFestival -leststring -lestools
	LIBS += -lestbase -lwinmm -mno-cygwin -lws2_32
	RC_FILE=audi.rc
	}

unix {
	INCLUDEPATH += 	/usr/include/vlc
	DEFINES += NVSPEECH_PATH=$$quote(\\\"/usr/share/novorado/nvspeech\\\")
	LIBS += -ltcl
	}

LIBS += -lvlc -lSDL -llm

################################################################################


################################################################################
LIBPATH += $$(INSTALL_LIB)

INSTALL_BIN = $$INSTALL_BIN
CUPER_SDK = $$CUPER_SDK

INCLUDEPATH += \
	. \
	../include \
	../widgetlib \
	../novolm/src/ \
	/usr/include/taglib \
	../parklib/ \
	../mixerlib/src \
	../speak \
	../fmpark/Firmware/src \
	../fmpark/Firmware \
        $(NOVORADO_DISTRIBS)/festival/src/include       \
        $(NOVORADO_DISTRIBS)/speech_tools/include


SOURCES = \
	audi.cpp

# platform code
win32 {
	INCLUDEPATH += /usr/win32/include
	}

unix {
	}

# -----------------------------------------------------------------------------

HEADERS = \
	audi.h

# -----------------------------------------------------------------------------

#DOC_LANGS = en
#TRANSLATIONS =                                  \
#    share/loganalyz/translations/de.ts

# -----------------------------------------------------------------------------

#UI_DIR = out
#IMAGES =                                        \
#    images/loganalyz_32.png                       \
#    images/stock_exit_16.png

#win32:RC_FILE =audi.rc

# -----------------------------------------------------------------------------

QT += network

CONFIG += warn_on qt thread exceptions debug_and_release
#CONFIG += warn_on qt exceptions thread
#LIBS += -lqassistantclient

# -----------------------------------------------------------------------------


# -----------------------------------------------------------------------------

CONFIG(debug,debug|release) {
    DEFINES += NOVORADO_DEBUG=1
    TARGET = audiD
    OBJECTS_DIR = obj_debug
    MOC_DIR = obj_debug
	win32:LIBS += -lparkD2 -lwidgetD1 -lhid -lnvspeak1
	unix:LIBS += -lparkD -lwidget -lhid -lnvspeak
    DESTDIR = $$(INSTALL_BIN)
} else  {
    TARGET = audi
    OBJECTS_DIR = obj_release
    MOC_DIR = obj_release
	win32:LIBS += -lpark2 -lwidget1 -lhid -lnvspeak1
	unix:LIBS += -lpark -lwidget -lhid -lnvspeak
    DESTDIR = $$(INSTALL_BIN)
}

DESTDIR = $$(INSTALL_BIN)

# -----------------------------------------------------------------------------

#
# make a tarball on Unix with "make tarball"
maketarball.target = tarball
maketarball.commands += pushd . &&
maketarball.commands += qmake parktronic.pro &&
maketarball.commands += make documentation &&
maketarball.commands += rm Makefile &&
maketarball.commands += cd /tmp &&
maketarball.commands += popd &&
QMAKE_EXTRA_UNIX_TARGETS += maketarball

#
# make documentation on Unix with "make documentation"
makedoc.target = documentation
makedoc.commands += cd doc/user &&
makedoc.commands += for i in $$DOC_LANGS ; do xsltproc -o \$$i/ parktronic.xsl \$$i/parktronic.xml ; done &&
makedoc.commands += cd -
QMAKE_EXTRA_UNIX_TARGETS += makedoc


# -----------------------------------------------------------------------------

#
# Installation

# install the documentation twice for convenience on Unix
unix : !mac {
        i_documentation.path = $$DOCDIR
        i_documentation.files += README COPYING ChangeLog
        i_documentation.files += doc/user/* doc/schematic
        INSTALLS += i_documentation
    }

i_userdoc.path = $$SHAREDIR/novorado/doc/
i_userdoc.files += doc/user/* doc/schematic
INSTALLS += i_userdoc

i_binary.path = $$BINDIR
i_binary.files += bin/parker
INSTALLS += i_binary

i_translation.path = $$SHAREDIR/novorado/translations
i_translation.files += share/novorado/translations/*.qm
INSTALLS += i_translation

i_share.path = $$SHAREDIR/novorado/
i_share.files += COPYING
INSTALLS += i_share

i_pixmaps.path = $$SHAREDIR/pixmaps
i_pixmaps.files += images/*.*
INSTALLS += i_pixmaps

RESOURCES += audi.qrc

