# Makefile for IOLib version 4.0

# Install prefixes:

BEOS_PREFIX = /boot/develop
CYGWIN_PREFIX = /usr
LINUX_PREFIX = /usr/local
QNX_PREFIX = /usr
BSD_PREFIX = /usr
SOLARIS_PREFIX = /usr
SGI_PREFIX = /usr
TRU64_PREFIX = /usr

ECHO = echo
MAKE = make
NMAKE = nmake

.PHONY: all
.SILENT:

all:
	$(ECHO) "This is IOLib version 4.0c"
	$(ECHO) "Written by Robert Kausch <robert.kausch@gmx.net>"
	$(ECHO) "This library is released under the terms of the GNU LGPL version 2."
	$(ECHO) ""
	$(ECHO) "Usage: make [target], make [target]-[un]install, make [target]-clean"
	$(ECHO) "The following targets are available:"
	$(ECHO) ""
	$(ECHO) "C targets              C++ targets             Objective-C targets"
	$(ECHO) ""
	$(ECHO) "bcc-c-static           bcc-c++-static"
	$(ECHO) "beos-c-static          beos-c++-static         beos-objc-static"
	$(ECHO) "beos-c-shared          beos-c++-shared         beos-objc-shared"
	$(ECHO) "cygwin-c-static        cygwin-c++-static       cygwin-objc-static"
	$(ECHO) "cygwin-c-shared        cygwin-c++-shared"
	$(ECHO) "dos-c-static           dos-c++-static          dos-objc-static"
	$(ECHO) "freebsd-c-static       freebsd-c++-static      freebsd-objc-static"
	$(ECHO) "freebsd-c-shared       freebsd-c++-shared      freebsd-objc-shared"
	$(ECHO) "lcc-c-static"
	$(ECHO) "lcc-c-shared"
	$(ECHO) "linux-c-static         linux-c++-static        linux-objc-static"
	$(ECHO) "linux-c-shared         linux-c++-shared        linux-objc-shared"
	$(ECHO) "msvc-c-static          msvc-c++-static"
	$(ECHO) "msvc-c-shared          msvc-c++-shared"
	$(ECHO) "qnx-c-static           qnx-c++-static          qnx-objc-static"
	$(ECHO) "qnx-c-shared           qnx-c++-shared          qnx-objc-shared"
	$(ECHO) "sgi-c-static           sgi-c++-static          sgi-objc-static"
	$(ECHO) "sgi-c-shared           sgi-c++-shared          sgi-objc-shared"
	$(ECHO) "solaris-c-static       solaris-c++-static      solaris-objc-static"
	$(ECHO) "solaris-c-shared       solaris-c++-shared      solaris-objc-shared"
	$(ECHO) "tru64-c-static"
	$(ECHO) "win32-c-static         win32-c++-static        win32-objc-static"
	$(ECHO) "win32-c-shared         win32-c++-shared"
	$(ECHO) ""

bcc-c-static:
	$(MAKE) -f makefiles/Makefile.bcc-static -DSRCEXT=.c -DINCLUDEEXT=-c -DIOLIBDIR=.\iolib-c

bcc-c-static-clean:
	$(MAKE) -f makefiles/Makefile.bcc-static -DSRCEXT=.c -DINCLUDEEXT=-c -DIOLIBDIR=.\iolib-c clean

bcc-c-static-install:
	$(ECHO) "There is no standard install method for Windows targets!"
	$(ECHO) "Copy the files to a directory of your choice!"

bcc-c-static-uninstall:
	$(ECHO) "There is no standard uninstall method for Windows targets!"
	$(ECHO) "You have to remove the files manually!"

beos-c-static:
	$(MAKE) -f makefiles/Makefile.beos-static SRCEXT=.c INCLUDEEXT=-c IOLIBDIR=./iolib-c

beos-c-static-clean:
	$(MAKE) -f makefiles/Makefile.beos-static SRCEXT=.c INCLUDEEXT=-c IOLIBDIR=./iolib-c clean

beos-c-static-install:
	$(MAKE) -f makefiles/Makefile.beos-static SRCEXT=.c INCLUDEEXT=-c IOLIBDIR=./iolib-c PREFIX=$(BEOS_PREFIX) install

beos-c-static-uninstall:
	$(MAKE) -f makefiles/Makefile.beos-static SRCEXT=.c INCLUDEEXT=-c IOLIBDIR=./iolib-c PREFIX=$(BEOS_PREFIX) uninstall

beos-c-shared:
	$(MAKE) -f makefiles/Makefile.beos-shared SRCEXT=.c INCLUDEEXT=-c IOLIBDIR=./iolib-c

beos-c-shared-clean:
	$(MAKE) -f makefiles/Makefile.beos-shared SRCEXT=.c INCLUDEEXT=-c IOLIBDIR=./iolib-c clean

beos-c-shared-install:
	$(MAKE) -f makefiles/Makefile.beos-shared SRCEXT=.c INCLUDEEXT=-c IOLIBDIR=./iolib-c PREFIX=$(BEOS_PREFIX) install

beos-c-shared-uninstall:
	$(MAKE) -f makefiles/Makefile.beos-shared SRCEXT=.c INCLUDEEXT=-c IOLIBDIR=./iolib-c PREFIX=$(BEOS_PREFIX) uninstall

cygwin-c-static:
	$(MAKE) -f makefiles/Makefile.cygwin-static SRCEXT=.c INCLUDEEXT=-c IOLIBDIR=./iolib-c

cygwin-c-static-clean:
	$(MAKE) -f makefiles/Makefile.cygwin-static SRCEXT=.c INCLUDEEXT=-c IOLIBDIR=./iolib-c clean

cygwin-c-static-install:
	$(MAKE) -f makefiles/Makefile.cygwin-static SRCEXT=.c INCLUDEEXT=-c IOLIBDIR=./iolib-c PREFIX=$(CYGWIN_PREFIX) install

cygwin-c-static-uninstall:
	$(MAKE) -f makefiles/Makefile.cygwin-static SRCEXT=.c INCLUDEEXT=-c IOLIBDIR=./iolib-c PREFIX=$(CYGWIN_PREFIX) uninstall

cygwin-c-shared:
	$(MAKE) -f makefiles/Makefile.cygwin-shared SRCEXT=.c INCLUDEEXT=-c IOLIBDIR=./iolib-c

cygwin-c-shared-clean:
	$(MAKE) -f makefiles/Makefile.cygwin-shared SRCEXT=.c INCLUDEEXT=-c IOLIBDIR=./iolib-c clean

cygwin-c-shared-install:
	$(MAKE) -f makefiles/Makefile.cygwin-shared SRCEXT=.c INCLUDEEXT=-c IOLIBDIR=./iolib-c PREFIX=$(CYGWIN_PREFIX) install

cygwin-c-shared-uninstall:
	$(MAKE) -f makefiles/Makefile.cygwin-shared SRCEXT=.c INCLUDEEXT=-c IOLIBDIR=./iolib-c PREFIX=$(CYGWIN_PREFIX) uninstall

dos-c-static:
	$(MAKE) -f makefiles/Makefile.dos-static SRCEXT=.c INCLUDEEXT=-c IOLIBDIR=./iolib-c

dos-c-static-clean:
	$(MAKE) -f makefiles/Makefile.dos-static SRCEXT=.c INCLUDEEXT=-c IOLIBDIR=./iolib-c clean

dos-c-static-install:
	$(ECHO) "There is no standard install method for DOS targets!"
	$(ECHO) "Copy the files to a directory of your choice!"

dos-c-static-uninstall:
	$(ECHO) "There is no standard uninstall method for DOS targets!"
	$(ECHO) "You have to remove the files manually!"

freebsd-c-static:
	$(MAKE) -f makefiles/Makefile.posix-static SRCEXT=.c INCLUDEEXT=-c IOLIBDIR=./iolib-c OTHERLIBS="-lstdc++ -lm"

freebsd-c-static-clean:
	$(MAKE) -f makefiles/Makefile.posix-static SRCEXT=.c INCLUDEEXT=-c IOLIBDIR=./iolib-c clean

freebsd-c-static-install:
	$(MAKE) -f makefiles/Makefile.posix-static SRCEXT=.c INCLUDEEXT=-c IOLIBDIR=./iolib-c PREFIX=$(BSD_PREFIX) install

freebsd-c-static-uninstall:
	$(MAKE) -f makefiles/Makefile.posix-static SRCEXT=.c INCLUDEEXT=-c IOLIBDIR=./iolib-c PREFIX=$(BSD_PREFIX) uninstall

freebsd-c-shared:
	$(MAKE) -f makefiles/Makefile.posix-shared SRCEXT=.c INCLUDEEXT=-c IOLIBDIR=./iolib-c OTHERLIBS="-lstdc++ -lm" SHAREDOPTS=-Wl,-soname,libiolib-c.so.5

freebsd-c-shared-clean:
	$(MAKE) -f makefiles/Makefile.posix-shared SRCEXT=.c INCLUDEEXT=-c IOLIBDIR=./iolib-c clean

freebsd-c-shared-install:
	$(MAKE) -f makefiles/Makefile.posix-shared SRCEXT=.c INCLUDEEXT=-c IOLIBDIR=./iolib-c PREFIX=$(BSD_PREFIX) install

freebsd-c-shared-uninstall:
	$(MAKE) -f makefiles/Makefile.posix-shared SRCEXT=.c INCLUDEEXT=-c IOLIBDIR=./iolib-c PREFIX=$(BSD_PREFIX) uninstall

lcc-c-static:
	@$(MAKE) -f makefiles/Makefile.lcc-static SRCEXT=.c INCLUDEEXT=-c IOLIBDIR=.\iolib-c

lcc-c-static-clean:
	@$(MAKE) -f makefiles/Makefile.lcc-static SRCEXT=.c INCLUDEEXT=-c IOLIBDIR=.\iolib-c clean

lcc-c-static-install:
	@$(ECHO) "There is no standard install method for Windows targets!"
	@$(ECHO) "Copy the files to a directory of your choice!"

lcc-c-static-uninstall:
	@$(ECHO) "There is no standard uninstall method for Windows targets!"
	@$(ECHO) "You have to remove the files manually!"

lcc-c-shared:
	@$(MAKE) -f makefiles/Makefile.lcc-shared SRCEXT=.c INCLUDEEXT=-c IOLIBDIR=.\iolib-c

lcc-c-shared-clean:
	@$(MAKE) -f makefiles/Makefile.lcc-shared SRCEXT=.c INCLUDEEXT=-c IOLIBDIR=.\iolib-c clean

lcc-c-shared-install:
	@$(ECHO) "There is no standard install method for Windows targets!"
	@$(ECHO) "Copy the files to a directory of your choice!"

lcc-c-shared-uninstall:
	@$(ECHO) "There is no standard uninstall method for Windows targets!"
	@$(ECHO) "You have to remove the files manually!"

linux-c-static:
	$(MAKE) -f makefiles/Makefile.posix-static SRCEXT=.c INCLUDEEXT=-c IOLIBDIR=./iolib-c OTHERLIBS="-lstdc++ -lm"

linux-c-static-clean:
	$(MAKE) -f makefiles/Makefile.posix-static SRCEXT=.c INCLUDEEXT=-c IOLIBDIR=./iolib-c clean

linux-c-static-install:
	$(MAKE) -f makefiles/Makefile.posix-static SRCEXT=.c INCLUDEEXT=-c IOLIBDIR=./iolib-c PREFIX=$(LINUX_PREFIX) install

linux-c-static-uninstall:
	$(MAKE) -f makefiles/Makefile.posix-static SRCEXT=.c INCLUDEEXT=-c IOLIBDIR=./iolib-c PREFIX=$(LINUX_PREFIX) uninstall

linux-c-shared:
	$(MAKE) -f makefiles/Makefile.posix-shared SRCEXT=.c INCLUDEEXT=-c IOLIBDIR=./iolib-c OTHERLIBS="-lstdc++ -lm" SHAREDOPTS=-Wl,-soname,libiolib-c.so.5

linux-c-shared-clean:
	$(MAKE) -f makefiles/Makefile.posix-shared SRCEXT=.c INCLUDEEXT=-c IOLIBDIR=./iolib-c clean

linux-c-shared-install:
	$(MAKE) -f makefiles/Makefile.posix-shared SRCEXT=.c INCLUDEEXT=-c IOLIBDIR=./iolib-c PREFIX=$(LINUX_PREFIX) install

linux-c-shared-uninstall:
	$(MAKE) -f makefiles/Makefile.posix-shared SRCEXT=.c INCLUDEEXT=-c IOLIBDIR=./iolib-c PREFIX=$(LINUX_PREFIX) uninstall

msvc-c-static:
	$(NMAKE) -f makefiles/Makefile.msvc-static SRCEXT=.c INCLUDEEXT=-c IOLIBDIR=./iolib-c

msvc-c-static-clean:
	$(NMAKE) -f makefiles/Makefile.msvc-static SRCEXT=.c INCLUDEEXT=-c IOLIBDIR=./iolib-c clean

msvc-c-static-install:
	$(ECHO) "There is no standard install method for Windows targets!"
	$(ECHO) "Copy the files to a directory of your choice!"

msvc-c-static-uninstall:
	$(ECHO) "There is no standard uninstall method for Windows targets!"
	$(ECHO) "You have to remove the files manually!"

msvc-c-shared:
	$(NMAKE) -f makefiles/Makefile.msvc-shared SRCEXT=.c INCLUDEEXT=-c IOLIBDIR=./iolib-c

msvc-c-shared-clean:
	$(NMAKE) -f makefiles/Makefile.msvc-shared SRCEXT=.c INCLUDEEXT=-c IOLIBDIR=./iolib-c clean

msvc-c-shared-install:
	$(ECHO) "There is no standard install method for Windows targets!"
	$(ECHO) "Copy the files to a directory of your choice!"

msvc-c-shared-uninstall:
	$(ECHO) "There is no standard uninstall method for Windows targets!"
	$(ECHO) "You have to remove the files manually!"

qnx-c-static:
	$(MAKE) -f makefiles/Makefile.posix-static SRCEXT=.c INCLUDEEXT=-c IOLIBDIR=./iolib-c OTHERLIBS="-lsocket -lstdc++ -lm"

qnx-c-static-clean:
	$(MAKE) -f makefiles/Makefile.posix-static SRCEXT=.c INCLUDEEXT=-c IOLIBDIR=./iolib-c clean

qnx-c-static-install:
	$(MAKE) -f makefiles/Makefile.posix-static SRCEXT=.c INCLUDEEXT=-c IOLIBDIR=./iolib-c PREFIX=$(QNX_PREFIX) install

qnx-c-static-uninstall:
	$(MAKE) -f makefiles/Makefile.posix-static SRCEXT=.c INCLUDEEXT=-c IOLIBDIR=./iolib-c PREFIX=$(QNX_PREFIX) uninstall

qnx-c-shared:
	$(MAKE) -f makefiles/Makefile.posix-shared SRCEXT=.c INCLUDEEXT=-c IOLIBDIR=./iolib-c OTHERLIBS="-lsocket -lstdc++ -lm" SHAREDOPTS=-Wl,-soname,libiolib-c.so.5

qnx-c-shared-clean:
	$(MAKE) -f makefiles/Makefile.posix-shared SRCEXT=.c INCLUDEEXT=-c IOLIBDIR=./iolib-c clean

qnx-c-shared-install:
	$(MAKE) -f makefiles/Makefile.posix-shared SRCEXT=.c INCLUDEEXT=-c IOLIBDIR=./iolib-c PREFIX=$(QNX_PREFIX) install

qnx-c-shared-uninstall:
	$(MAKE) -f makefiles/Makefile.posix-shared SRCEXT=.c INCLUDEEXT=-c IOLIBDIR=./iolib-c PREFIX=$(QNX_PREFIX) uninstall

sgi-c-static:

	$(MAKE) -f makefiles/Makefile.posix-static SRCEXT=.c INCLUDEEXT=-c IOLIBDIR=./iolib-c OTHERLIBS=-lm STRIP_OPTS=-f



sgi-c-static-clean:

	$(MAKE) -f makefiles/Makefile.posix-static SRCEXT=.c INCLUDEEXT=-c IOLIBDIR=./iolib-c clean



sgi-c-static-install:

	$(MAKE) -f makefiles/Makefile.posix-static SRCEXT=.c INCLUDEEXT=-c IOLIBDIR=./iolib-c PREFIX=$(SGI_PREFIX) install



sgi-c-static-uninstall:

	$(MAKE) -f makefiles/Makefile.posix-static SRCEXT=.c INCLUDEEXT=-c IOLIBDIR=./iolib-c PREFIX=$(SGI_PREFIX) uninstall



sgi-c-shared:

	$(MAKE) -f makefiles/Makefile.posix-shared SRCEXT=.c INCLUDEEXT=-c IOLIBDIR=./iolib-c OTHERLIBS=-lm STRIP_OPTS=-f

sgi-c-shared-clean:

	$(MAKE) -f makefiles/Makefile.posix-shared SRCEXT=.c INCLUDEEXT=-c IOLIBDIR=./iolib-c clean



sgi-c-shared-install:

	$(MAKE) -f makefiles/Makefile.posix-shared SRCEXT=.c INCLUDEEXT=-c IOLIBDIR=./iolib-c PREFIX=$(SGI_PREFIX) install



sgi-c-shared-uninstall:
	$(MAKE) -f makefiles/Makefile.posix-shared SRCEXT=.c INCLUDEEXT=-c IOLIBDIR=./iolib-c PREFIX=$(SGI_PREFIX) uninstall


solaris-c-static:
	$(MAKE) -f makefiles/Makefile.posix-static SRCEXT=.c INCLUDEEXT=-c IOLIBDIR=./iolib-c OTHERLIBS="-lsocket -lnsl -lstdc++ -lm"

solaris-c-static-clean:
	$(MAKE) -f makefiles/Makefile.posix-static SRCEXT=.c INCLUDEEXT=-c IOLIBDIR=./iolib-c clean

solaris-c-static-install:
	$(MAKE) -f makefiles/Makefile.posix-static SRCEXT=.c INCLUDEEXT=-c IOLIBDIR=./iolib-c PREFIX=$(SOLARIS_PREFIX) install

solaris-c-static-uninstall:
	$(MAKE) -f makefiles/Makefile.posix-static SRCEXT=.c INCLUDEEXT=-c IOLIBDIR=./iolib-c PREFIX=$(SOLARIS_PREFIX) uninstall

solaris-c-shared:
	$(MAKE) -f makefiles/Makefile.posix-shared SRCEXT=.c INCLUDEEXT=-c IOLIBDIR=./iolib-c OTHERLIBS="-lsocket -lnsl -lstdc++ -lm"

solaris-c-shared-clean:
	$(MAKE) -f makefiles/Makefile.posix-shared SRCEXT=.c INCLUDEEXT=-c IOLIBDIR=./iolib-c clean

solaris-c-shared-install:
	$(MAKE) -f makefiles/Makefile.posix-shared SRCEXT=.c INCLUDEEXT=-c IOLIBDIR=./iolib-c PREFIX=$(SOLARIS_PREFIX) install

solaris-c-shared-uninstall:
	$(MAKE) -f makefiles/Makefile.posix-shared SRCEXT=.c INCLUDEEXT=-c IOLIBDIR=./iolib-c PREFIX=$(SOLARIS_PREFIX) uninstall

tru64-c-static:
	$(MAKE) -f makefiles/Makefile.tru64-static SRCEXT=.c INCLUDEEXT=-c IOLIBDIR=./iolib-c OTHERLIBS=-lm

tru64-c-static-clean:
	$(MAKE) -f makefiles/Makefile.tru64-static SRCEXT=.c INCLUDEEXT=-c IOLIBDIR=./iolib-c clean

tru64-c-static-install:
	$(MAKE) -f makefiles/Makefile.tru64-static SRCEXT=.c INCLUDEEXT=-c IOLIBDIR=./iolib-c PREFIX=$(TRU64_PREFIX) install

tru64-c-static-uninstall:
	$(MAKE) -f makefiles/Makefile.tru64-static SRCEXT=.c INCLUDEEXT=-c IOLIBDIR=./iolib-c PREFIX=$(TRU64_PREFIX) uninstall

win32-c-static:
	$(MAKE) -f makefiles/Makefile.win32-static SRCEXT=.c INCLUDEEXT=-c IOLIBDIR=./iolib-c

win32-c-static-clean:
	$(MAKE) -f makefiles/Makefile.win32-static SRCEXT=.c INCLUDEEXT=-c IOLIBDIR=./iolib-c clean

win32-c-static-install:
	$(ECHO) "There is no standard install method for Windows targets!"
	$(ECHO) "Copy the files to a directory of your choice!"

win32-c-static-uninstall:
	$(ECHO) "There is no standard uninstall method for Windows targets!"
	$(ECHO) "You have to remove the files manually!"

win32-c-shared:
	$(MAKE) -f makefiles/Makefile.win32-shared SRCEXT=.c INCLUDEEXT=-c IOLIBDIR=./iolib-c

win32-c-shared-clean:
	$(MAKE) -f makefiles/Makefile.win32-shared SRCEXT=.c INCLUDEEXT=-c IOLIBDIR=./iolib-c clean

win32-c-shared-install:
	$(ECHO) "There is no standard install method for Windows targets!"
	$(ECHO) "Copy the files to a directory of your choice!"

win32-c-shared-uninstall:
	$(ECHO) "There is no standard uninstall method for Windows targets!"
	$(ECHO) "You have to remove the files manually!"

bcc-c++-static:
	$(MAKE) -f makefiles/Makefile.bcc-static -DSRCEXT=.cpp -DINCLUDEEXT=-cxx -DIOLIBDIR=.\iolib-cxx

bcc-c++-static-clean:
	$(MAKE) -f makefiles/Makefile.bcc-static -DSRCEXT=.cpp -DINCLUDEEXT=-cxx -DIOLIBDIR=.\iolib-cxx clean

bcc-c++-static-install:
	$(ECHO) "There is no standard install method for Windows targets!"
	$(ECHO) "Copy the files to a directory of your choice!"

bcc-c++-static-uninstall:
	$(ECHO) "There is no standard uninstall method for Windows targets!"
	$(ECHO) "You have to remove the files manually!"

beos-c++-static:
	$(MAKE) -f makefiles/Makefile.beos-static SRCEXT=.cpp INCLUDEEXT=-cxx IOLIBDIR=./iolib-cxx

beos-c++-static-clean:
	$(MAKE) -f makefiles/Makefile.beos-static SRCEXT=.cpp INCLUDEEXT=-cxx IOLIBDIR=./iolib-cxx clean

beos-c++-static-install:
	$(MAKE) -f makefiles/Makefile.beos-static SRCEXT=.cpp INCLUDEEXT=-cxx IOLIBDIR=./iolib-cxx PREFIX=$(BEOS_PREFIX) install

beos-c++-static-uninstall:
	$(MAKE) -f makefiles/Makefile.beos-static SRCEXT=.cpp INCLUDEEXT=-cxx IOLIBDIR=./iolib-cxx PREFIX=$(BEOS_PREFIX) uninstall

beos-c++-shared:
	$(MAKE) -f makefiles/Makefile.beos-shared SRCEXT=.cpp INCLUDEEXT=-cxx IOLIBDIR=./iolib-cxx

beos-c++-shared-clean:
	$(MAKE) -f makefiles/Makefile.beos-shared SRCEXT=.cpp INCLUDEEXT=-cxx IOLIBDIR=./iolib-cxx clean

beos-c++-shared-install:
	$(MAKE) -f makefiles/Makefile.beos-shared SRCEXT=.cpp INCLUDEEXT=-cxx IOLIBDIR=./iolib-cxx PREFIX=$(BEOS_PREFIX) install

beos-c++-shared-uninstall:
	$(MAKE) -f makefiles/Makefile.beos-shared SRCEXT=.cpp INCLUDEEXT=-cxx IOLIBDIR=./iolib-cxx PREFIX=$(BEOS_PREFIX) uninstall

cygwin-c++-static:
	$(MAKE) -f makefiles/Makefile.cygwin-static SRCEXT=.cpp INCLUDEEXT=-cxx IOLIBDIR=./iolib-cxx

cygwin-c++-static-clean:
	$(MAKE) -f makefiles/Makefile.cygwin-static SRCEXT=.cpp INCLUDEEXT=-cxx IOLIBDIR=./iolib-cxx clean

cygwin-c++-static-install:
	$(MAKE) -f makefiles/Makefile.cygwin-static SRCEXT=.cpp INCLUDEEXT=-cxx IOLIBDIR=./iolib-cxx PREFIX=$(CYGWIN_PREFIX) install

cygwin-c++-static-uninstall:
	$(MAKE) -f makefiles/Makefile.cygwin-static SRCEXT=.cpp INCLUDEEXT=-cxx IOLIBDIR=./iolib-cxx PREFIX=$(CYGWIN_PREFIX) uninstall

cygwin-c++-shared:
	$(MAKE) -f makefiles/Makefile.cygwin-shared SRCEXT=.cpp INCLUDEEXT=-cxx IOLIBDIR=./iolib-cxx

cygwin-c++-shared-clean:
	$(MAKE) -f makefiles/Makefile.cygwin-shared SRCEXT=.cpp INCLUDEEXT=-cxx IOLIBDIR=./iolib-cxx clean

cygwin-c++-shared-install:
	$(MAKE) -f makefiles/Makefile.cygwin-shared SRCEXT=.cpp INCLUDEEXT=-cxx IOLIBDIR=./iolib-cxx PREFIX=$(CYGWIN_PREFIX) install

cygwin-c++-shared-uninstall:
	$(MAKE) -f makefiles/Makefile.cygwin-shared SRCEXT=.cpp INCLUDEEXT=-cxx IOLIBDIR=./iolib-cxx PREFIX=$(CYGWIN_PREFIX) uninstall

dos-c++-static:
	$(MAKE) -f makefiles/Makefile.dos-static SRCEXT=.cpp INCLUDEEXT=-cxx IOLIBDIR=./iolib-cxx

dos-c++-static-clean:
	$(MAKE) -f makefiles/Makefile.dos-static SRCEXT=.cpp INCLUDEEXT=-cxx IOLIBDIR=./iolib-cxx clean

dos-c++-static-install:
	$(ECHO) "There is no standard install method for DOS targets!"
	$(ECHO) "Copy the files to a directory of your choice!"

dos-c++-static-uninstall:
	$(ECHO) "There is no standard uninstall method for DOS targets!"
	$(ECHO) "You have to remove the files manually!"

freebsd-c++-static:
	$(MAKE) -f makefiles/Makefile.posix-static SRCEXT=.cpp INCLUDEEXT=-cxx IOLIBDIR=./iolib-cxx OTHERLIBS="-lstdc++ -lm"

freebsd-c++-static-clean:
	$(MAKE) -f makefiles/Makefile.posix-static SRCEXT=.cpp INCLUDEEXT=-cxx IOLIBDIR=./iolib-cxx clean

freebsd-c++-static-install:
	$(MAKE) -f makefiles/Makefile.posix-static SRCEXT=.cpp INCLUDEEXT=-cxx IOLIBDIR=./iolib-cxx PREFIX=$(BSD_PREFIX) install

freebsd-c++-static-uninstall:
	$(MAKE) -f makefiles/Makefile.posix-static SRCEXT=.cpp INCLUDEEXT=-cxx IOLIBDIR=./iolib-cxx PREFIX=$(BSD_PREFIX) uninstall

freebsd-c++-shared:
	$(MAKE) -f makefiles/Makefile.posix-shared SRCEXT=.cpp INCLUDEEXT=-cxx IOLIBDIR=./iolib-cxx OTHERLIBS="-lstdc++ -lm" SHAREDOPTS=-Wl,-soname,libiolib-cxx.so.5

freebsd-c++-shared-clean:
	$(MAKE) -f makefiles/Makefile.posix-shared SRCEXT=.cpp INCLUDEEXT=-cxx IOLIBDIR=./iolib-cxx clean

freebsd-c++-shared-install:
	$(MAKE) -f makefiles/Makefile.posix-shared SRCEXT=.cpp INCLUDEEXT=-cxx IOLIBDIR=./iolib-cxx PREFIX=$(BSD_PREFIX) install

freebsd-c++-shared-uninstall:
	$(MAKE) -f makefiles/Makefile.posix-shared SRCEXT=.cpp INCLUDEEXT=-cxx IOLIBDIR=./iolib-cxx PREFIX=$(BSD_PREFIX) uninstall

linux-c++-static:
	$(MAKE) -f makefiles/Makefile.posix-static SRCEXT=.cpp INCLUDEEXT=-cxx IOLIBDIR=./iolib-cxx OTHERLIBS="-lstdc++ -lm"

linux-c++-static-clean:
	$(MAKE) -f makefiles/Makefile.posix-static SRCEXT=.cpp INCLUDEEXT=-cxx IOLIBDIR=./iolib-cxx clean

linux-c++-static-install:
	$(MAKE) -f makefiles/Makefile.posix-static SRCEXT=.cpp INCLUDEEXT=-cxx IOLIBDIR=./iolib-cxx PREFIX=$(LINUX_PREFIX) install

linux-c++-static-uninstall:
	$(MAKE) -f makefiles/Makefile.posix-static SRCEXT=.cpp INCLUDEEXT=-cxx IOLIBDIR=./iolib-cxx PREFIX=$(LINUX_PREFIX) uninstall

linux-c++-shared:
	$(MAKE) -f makefiles/Makefile.posix-shared SRCEXT=.cpp INCLUDEEXT=-cxx IOLIBDIR=./iolib-cxx OTHERLIBS="-lstdc++ -lm" SHAREDOPTS=-Wl,-soname,libiolib-cxx.so.5

linux-c++-shared-clean:
	$(MAKE) -f makefiles/Makefile.posix-shared SRCEXT=.cpp INCLUDEEXT=-cxx IOLIBDIR=./iolib-cxx clean

linux-c++-shared-install:
	$(MAKE) -f makefiles/Makefile.posix-shared SRCEXT=.cpp INCLUDEEXT=-cxx IOLIBDIR=./iolib-cxx PREFIX=$(LINUX_PREFIX) install

linux-c++-shared-uninstall:
	$(MAKE) -f makefiles/Makefile.posix-shared SRCEXT=.cpp INCLUDEEXT=-cxx IOLIBDIR=./iolib-cxx PREFIX=$(LINUX_PREFIX) uninstall

msvc-c++-static:
	$(NMAKE) -f makefiles/Makefile.msvc-static SRCEXT=.cpp INCLUDEEXT=-cxx IOLIBDIR=./iolib-cxx

msvc-c++-static-clean:
	$(NMAKE) -f makefiles/Makefile.msvc-static SRCEXT=.cpp INCLUDEEXT=-cxx IOLIBDIR=./iolib-cxx clean

msvc-c++-static-install:
	$(ECHO) "There is no standard install method for Windows targets!"
	$(ECHO) "Copy the files to a directory of your choice!"

msvc-c++-static-uninstall:
	$(ECHO) "There is no standard uninstall method for Windows targets!"
	$(ECHO) "You have to remove the files manually!"

msvc-c++-shared:
	$(NMAKE) -f makefiles/Makefile.msvc-shared SRCEXT=.cpp INCLUDEEXT=-cxx IOLIBDIR=./iolib-cxx

msvc-c++-shared-clean:
	$(NMAKE) -f makefiles/Makefile.msvc-shared SRCEXT=.cpp INCLUDEEXT=-cxx IOLIBDIR=./iolib-cxx clean

msvc-c++-shared-install:
	$(ECHO) "There is no standard install method for Windows targets!"
	$(ECHO) "Copy the files to a directory of your choice!"

msvc-c++-shared-uninstall:
	$(ECHO) "There is no standard uninstall method for Windows targets!"
	$(ECHO) "You have to remove the files manually!"

qnx-c++-static:
	$(MAKE) -f makefiles/Makefile.posix-static SRCEXT=.cpp INCLUDEEXT=-cxx IOLIBDIR=./iolib-cxx OTHERLIBS="-lsocket -lstdc++ -lm"

qnx-c++-static-clean:
	$(MAKE) -f makefiles/Makefile.posix-static SRCEXT=.cpp INCLUDEEXT=-cxx IOLIBDIR=./iolib-cxx clean

qnx-c++-static-install:
	$(MAKE) -f makefiles/Makefile.posix-static SRCEXT=.cpp INCLUDEEXT=-cxx IOLIBDIR=./iolib-cxx PREFIX=$(QNX_PREFIX) install

qnx-c++-static-uninstall:
	$(MAKE) -f makefiles/Makefile.posix-static SRCEXT=.cpp INCLUDEEXT=-cxx IOLIBDIR=./iolib-cxx PREFIX=$(QNX_PREFIX) uninstall

qnx-c++-shared:
	$(MAKE) -f makefiles/Makefile.posix-shared SRCEXT=.cpp INCLUDEEXT=-cxx IOLIBDIR=./iolib-cxx OTHERLIBS="-lsocket -lstdc++ -lm" SHAREDOPTS=-Wl,-soname,libiolib-cxx.so.5

qnx-c++-shared-clean:
	$(MAKE) -f makefiles/Makefile.posix-shared SRCEXT=.cpp INCLUDEEXT=-cxx IOLIBDIR=./iolib-cxx clean

qnx-c++-shared-install:
	$(MAKE) -f makefiles/Makefile.posix-shared SRCEXT=.cpp INCLUDEEXT=-cxx IOLIBDIR=./iolib-cxx PREFIX=$(QNX_PREFIX) install

qnx-c++-shared-uninstall:
	$(MAKE) -f makefiles/Makefile.posix-shared SRCEXT=.cpp INCLUDEEXT=-cxx IOLIBDIR=./iolib-cxx PREFIX=$(QNX_PREFIX) uninstall

sgi-c++-static:
	$(MAKE) -f makefiles/Makefile.posix-static SRCEXT=.cpp INCLUDEEXT=-cxx IOLIBDIR=./iolib-cxx OTHERLIBS=-lm

sgi-c++-static-clean:
	$(MAKE) -f makefiles/Makefile.posix-static SRCEXT=.cpp INCLUDEEXT=-cxx IOLIBDIR=./iolib-cxx clean

sgi-c++-static-install:
	$(MAKE) -f makefiles/Makefile.posix-static SRCEXT=.cpp INCLUDEEXT=-cxx IOLIBDIR=./iolib-cxx PREFIX=$(SGI_PREFIX) install

sgi-c++-static-uninstall:
	$(MAKE) -f makefiles/Makefile.posix-static SRCEXT=.cpp INCLUDEEXT=-cxx IOLIBDIR=./iolib-cxx PREFIX=$(SGI_PREFIX) uninstall

sgi-c++-shared:
	$(MAKE) -f makefiles/Makefile.posix-shared SRCEXT=.cpp INCLUDEEXT=-cxx IOLIBDIR=./iolib-cxx OTHERLIBS=-lm

sgi-c++-shared-clean:
	$(MAKE) -f makefiles/Makefile.posix-shared SRCEXT=.cpp INCLUDEEXT=-cxx IOLIBDIR=./iolib-cxx clean

sgi-c++-shared-install:
	$(MAKE) -f makefiles/Makefile.posix-shared SRCEXT=.cpp INCLUDEEXT=-cxx IOLIBDIR=./iolib-cxx PREFIX=$(SGI_PREFIX) install

sgi-c++-shared-uninstall:
	$(MAKE) -f makefiles/Makefile.posix-shared SRCEXT=.cpp INCLUDEEXT=-cxx IOLIBDIR=./iolib-cxx PREFIX=$(SGI_PREFIX) uninstall

solaris-c++-static:
	$(MAKE) -f makefiles/Makefile.posix-static SRCEXT=.cpp INCLUDEEXT=-cxx IOLIBDIR=./iolib-cxx OTHERLIBS="-lsocket -lnsl -lstdc++ -lm"

solaris-c++-static-clean:
	$(MAKE) -f makefiles/Makefile.posix-static SRCEXT=.cpp INCLUDEEXT=-cxx IOLIBDIR=./iolib-cxx clean

solaris-c++-static-install:
	$(MAKE) -f makefiles/Makefile.posix-static SRCEXT=.cpp INCLUDEEXT=-cxx IOLIBDIR=./iolib-cxx PREFIX=$(SOLARIS_PREFIX) install

solaris-c++-static-uninstall:
	$(MAKE) -f makefiles/Makefile.posix-static SRCEXT=.cpp INCLUDEEXT=-cxx IOLIBDIR=./iolib-cxx PREFIX=$(SOLARIS_PREFIX) uninstall

solaris-c++-shared:
	$(MAKE) -f makefiles/Makefile.posix-shared SRCEXT=.cpp INCLUDEEXT=-cxx IOLIBDIR=./iolib-cxx OTHERLIBS="-lsocket -lnsl -lstdc++ -lm"

solaris-c++-shared-clean:
	$(MAKE) -f makefiles/Makefile.posix-shared SRCEXT=.cpp INCLUDEEXT=-cxx IOLIBDIR=./iolib-cxx clean

solaris-c++-shared-install:
	$(MAKE) -f makefiles/Makefile.posix-shared SRCEXT=.cpp INCLUDEEXT=-cxx IOLIBDIR=./iolib-cxx PREFIX=$(SOLARIS_PREFIX) install

solaris-c++-shared-uninstall:
	$(MAKE) -f makefiles/Makefile.posix-shared SRCEXT=.cpp INCLUDEEXT=-cxx IOLIBDIR=./iolib-cxx PREFIX=$(SOLARIS_PREFIX) uninstall

win32-c++-static:
	$(MAKE) -f makefiles/Makefile.win32-static SRCEXT=.cpp INCLUDEEXT=-cxx IOLIBDIR=./iolib-cxx

win32-c++-static-clean:
	$(MAKE) -f makefiles/Makefile.win32-static SRCEXT=.cpp INCLUDEEXT=-cxx IOLIBDIR=./iolib-cxx clean

win32-c++-static-install:
	$(ECHO) "There is no standard install method for Windows targets!"
	$(ECHO) "Copy the files to a directory of your choice!"

win32-c++-static-uninstall:
	$(ECHO) "There is no standard uninstall method for Windows targets!"
	$(ECHO) "You have to remove the files manually!"

win32-c++-shared:
	$(MAKE) -f makefiles/Makefile.win32-shared SRCEXT=.cpp INCLUDEEXT=-cxx IOLIBDIR=./iolib-cxx

win32-c++-shared-clean:
	$(MAKE) -f makefiles/Makefile.win32-shared SRCEXT=.cpp INCLUDEEXT=-cxx IOLIBDIR=./iolib-cxx clean

win32-c++-shared-install:
	$(ECHO) "There is no standard install method for Windows targets!"
	$(ECHO) "Copy the files to a directory of your choice!"

win32-c++-shared-uninstall:
	$(ECHO) "There is no standard uninstall method for Windows targets!"
	$(ECHO) "You have to remove the files manually!"

beos-objc-static:
	$(MAKE) -f makefiles/Makefile.beos-static SRCEXT=.m INCLUDEEXT=-objc IOLIBDIR=./iolib-objc OTHERLIBS=-lobjc

beos-objc-static-clean:
	$(MAKE) -f makefiles/Makefile.beos-static SRCEXT=.m INCLUDEEXT=-objc IOLIBDIR=./iolib-objc clean

beos-objc-static-install:
	$(MAKE) -f makefiles/Makefile.beos-static SRCEXT=.m INCLUDEEXT=-objc IOLIBDIR=./iolib-objc PREFIX=$(BEOS_PREFIX) install

beos-objc-static-uninstall:
	$(MAKE) -f makefiles/Makefile.beos-static SRCEXT=.m INCLUDEEXT=-objc IOLIBDIR=./iolib-objc PREFIX=$(BEOS_PREFIX) uninstall

beos-objc-shared:
	$(MAKE) -f makefiles/Makefile.beos-shared SRCEXT=.m INCLUDEEXT=-objc IOLIBDIR=./iolib-objc OTHERLIBS=-lobjc

beos-objc-shared-clean:
	$(MAKE) -f makefiles/Makefile.beos-shared SRCEXT=.m INCLUDEEXT=-objc IOLIBDIR=./iolib-objc clean

beos-objc-shared-install:
	$(MAKE) -f makefiles/Makefile.beos-shared SRCEXT=.m INCLUDEEXT=-objc IOLIBDIR=./iolib-objc PREFIX=$(BEOS_PREFIX) install

beos-objc-shared-uninstall:
	$(MAKE) -f makefiles/Makefile.beos-shared SRCEXT=.m INCLUDEEXT=-objc IOLIBDIR=./iolib-objc PREFIX=$(BEOS_PREFIX) uninstall

cygwin-objc-static:
	$(MAKE) -f makefiles/Makefile.cygwin-static SRCEXT=.m INCLUDEEXT=-objc IOLIBDIR=./iolib-objc OTHERLIBS=-lobjc

cygwin-objc-static-clean:
	$(MAKE) -f makefiles/Makefile.cygwin-static SRCEXT=.m INCLUDEEXT=-objc IOLIBDIR=./iolib-objc clean

cygwin-objc-static-install:
	$(MAKE) -f makefiles/Makefile.cygwin-static SRCEXT=.m INCLUDEEXT=-objc IOLIBDIR=./iolib-objc PREFIX=$(CYGWIN_PREFIX) install

cygwin-objc-static-uninstall:
	$(MAKE) -f makefiles/Makefile.cygwin-static SRCEXT=.m INCLUDEEXT=-objc IOLIBDIR=./iolib-objc PREFIX=$(CYGWIN_PREFIX) uninstall

dos-objc-static:
	$(MAKE) -f makefiles/Makefile.dos-static SRCEXT=.m INCLUDEEXT=-objc IOLIBDIR=./iolib-objc OTHERLIBS=-lobjc

dos-objc-static-clean:
	$(MAKE) -f makefiles/Makefile.dos-static SRCEXT=.m INCLUDEEXT=-objc IOLIBDIR=./iolib-objc clean

dos-objc-static-install:
	$(ECHO) "There is no standard install method for DOS targets!"
	$(ECHO) "Copy the files to a directory of your choice!"

dos-objc-static-uninstall:
	$(ECHO) "There is no standard uninstall method for DOS targets!"
	$(ECHO) "You have to remove the files manually!"

freebsd-objc-static:
	$(MAKE) -f makefiles/Makefile.posix-static SRCEXT=.m INCLUDEEXT=-objc IOLIBDIR=./iolib-objc OTHERLIBS="-lobjc -lstdc++ -lm"

freebsd-objc-static-clean:
	$(MAKE) -f makefiles/Makefile.posix-static SRCEXT=.m INCLUDEEXT=-objc IOLIBDIR=./iolib-objc clean

freebsd-objc-static-install:
	$(MAKE) -f makefiles/Makefile.posix-static SRCEXT=.m INCLUDEEXT=-objc IOLIBDIR=./iolib-objc PREFIX=$(BSD_PREFIX) install

freebsd-objc-static-uninstall:
	$(MAKE) -f makefiles/Makefile.posix-static SRCEXT=.m INCLUDEEXT=-objc IOLIBDIR=./iolib-objc PREFIX=$(BSD_PREFIX) uninstall

freebsd-objc-shared:
	$(MAKE) -f makefiles/Makefile.posix-shared SRCEXT=.m INCLUDEEXT=-objc IOLIBDIR=./iolib-objc OTHERLIBS="-lobjc -lstdc++ -lm" SHAREDOPTS=-Wl,-soname,libiolib-objc.so.5

freebsd-objc-shared-clean:
	$(MAKE) -f makefiles/Makefile.posix-shared SRCEXT=.m INCLUDEEXT=-objc IOLIBDIR=./iolib-objc clean

freebsd-objc-shared-install:
	$(MAKE) -f makefiles/Makefile.posix-shared SRCEXT=.m INCLUDEEXT=-objc IOLIBDIR=./iolib-objc PREFIX=$(BSD_PREFIX) install

freebsd-objc-shared-uninstall:
	$(MAKE) -f makefiles/Makefile.posix-shared SRCEXT=.m INCLUDEEXT=-objc IOLIBDIR=./iolib-objc PREFIX=$(BSD_PREFIX) uninstall

linux-objc-static:
	$(MAKE) -f makefiles/Makefile.posix-static SRCEXT=.m INCLUDEEXT=-objc IOLIBDIR=./iolib-objc OTHERLIBS="-lobjc -lstdc++ -lm"

linux-objc-static-clean:
	$(MAKE) -f makefiles/Makefile.posix-static SRCEXT=.m INCLUDEEXT=-objc IOLIBDIR=./iolib-objc clean

linux-objc-static-install:
	$(MAKE) -f makefiles/Makefile.posix-static SRCEXT=.m INCLUDEEXT=-objc IOLIBDIR=./iolib-objc PREFIX=$(LINUX_PREFIX) install

linux-objc-static-uninstall:
	$(MAKE) -f makefiles/Makefile.posix-static SRCEXT=.m INCLUDEEXT=-objc IOLIBDIR=./iolib-objc PREFIX=$(LINUX_PREFIX) uninstall

linux-objc-shared:
	$(MAKE) -f makefiles/Makefile.posix-shared SRCEXT=.m INCLUDEEXT=-objc IOLIBDIR=./iolib-objc OTHERLIBS="-lobjc -lstdc++ -lm" SHAREDOPTS=-Wl,-soname,libiolib-objc.so.5

linux-objc-shared-clean:
	$(MAKE) -f makefiles/Makefile.posix-shared SRCEXT=.m INCLUDEEXT=-objc IOLIBDIR=./iolib-objc clean

linux-objc-shared-install:
	$(MAKE) -f makefiles/Makefile.posix-shared SRCEXT=.m INCLUDEEXT=-objc IOLIBDIR=./iolib-objc PREFIX=$(LINUX_PREFIX) install

linux-objc-shared-uninstall:
	$(MAKE) -f makefiles/Makefile.posix-shared SRCEXT=.m INCLUDEEXT=-objc IOLIBDIR=./iolib-objc PREFIX=$(LINUX_PREFIX) uninstall

qnx-objc-static:
	$(MAKE) -f makefiles/Makefile.posix-static SRCEXT=.m INCLUDEEXT=-objc IOLIBDIR=./iolib-objc OTHERLIBS="-lobjc -lsocket -lstdc++ -lm"

qnx-objc-static-clean:
	$(MAKE) -f makefiles/Makefile.posix-static SRCEXT=.m INCLUDEEXT=-objc IOLIBDIR=./iolib-objc clean

qnx-objc-static-install:
	$(MAKE) -f makefiles/Makefile.posix-static SRCEXT=.m INCLUDEEXT=-objc IOLIBDIR=./iolib-objc PREFIX=$(QNX_PREFIX) install

qnx-objc-static-uninstall:
	$(MAKE) -f makefiles/Makefile.posix-static SRCEXT=.m INCLUDEEXT=-objc IOLIBDIR=./iolib-objc PREFIX=$(QNX_PREFIX) uninstall

qnx-objc-shared:
	$(MAKE) -f makefiles/Makefile.posix-shared SRCEXT=.m INCLUDEEXT=-objc IOLIBDIR=./iolib-objc OTHERLIBS="-lobjc -lsocket -lstdc++ -lm" SHAREDOPTS=-Wl,-soname,libiolib-objc.so.5

qnx-objc-shared-clean:
	$(MAKE) -f makefiles/Makefile.posix-shared SRCEXT=.m INCLUDEEXT=-objc IOLIBDIR=./iolib-objc clean

qnx-objc-shared-install:
	$(MAKE) -f makefiles/Makefile.posix-shared SRCEXT=.m INCLUDEEXT=-objc IOLIBDIR=./iolib-objc PREFIX=$(QNX_PREFIX) install

qnx-objc-shared-uninstall:
	$(MAKE) -f makefiles/Makefile.posix-shared SRCEXT=.m INCLUDEEXT=-objc IOLIBDIR=./iolib-objc PREFIX=$(QNX_PREFIX) uninstall

sgi-objc-static:
	$(MAKE) -f makefiles/Makefile.posix-static SRCEXT=.m INCLUDEEXT=-objc IOLIBDIR=./iolib-objc OTHERLIBS="-lobjc -lsocket -lnsl -lstdc++ -lm"

sgi-objc-static-clean:
	$(MAKE) -f makefiles/Makefile.posix-static SRCEXT=.m INCLUDEEXT=-objc IOLIBDIR=./iolib-objc clean

sgi-objc-static-install:
	$(MAKE) -f makefiles/Makefile.posix-static SRCEXT=.m INCLUDEEXT=-objc IOLIBDIR=./iolib-objc PREFIX=$(SGI_PREFIX) install

sgi-objc-static-uninstall:
	$(MAKE) -f makefiles/Makefile.posix-static SRCEXT=.m INCLUDEEXT=-objc IOLIBDIR=./iolib-objc PREFIX=$(SGI_PREFIX) uninstall

sgi-objc-shared:
	$(MAKE) -f makefiles/Makefile.posix-shared SRCEXT=.m INCLUDEEXT=-objc IOLIBDIR=./iolib-objc OTHERLIBS="-lobjc -lm"

sgi-objc-shared-clean:
	$(MAKE) -f makefiles/Makefile.posix-shared SRCEXT=.m INCLUDEEXT=-objc IOLIBDIR=./iolib-objc clean

sgi-objc-shared-install:
	$(MAKE) -f makefiles/Makefile.posix-shared SRCEXT=.m INCLUDEEXT=-objc IOLIBDIR=./iolib-objc PREFIX=$(SGI_PREFIX) install

sgi-objc-shared-uninstall:
	$(MAKE) -f makefiles/Makefile.posix-shared SRCEXT=.m INCLUDEEXT=-objc IOLIBDIR=./iolib-objc PREFIX=$(SGI_PREFIX) uninstall

solaris-objc-static:
	$(MAKE) -f makefiles/Makefile.posix-static SRCEXT=.m INCLUDEEXT=-objc IOLIBDIR=./iolib-objc OTHERLIBS="-lobjc -lm"

solaris-objc-static-clean:
	$(MAKE) -f makefiles/Makefile.posix-static SRCEXT=.m INCLUDEEXT=-objc IOLIBDIR=./iolib-objc clean

solaris-objc-static-install:
	$(MAKE) -f makefiles/Makefile.posix-static SRCEXT=.m INCLUDEEXT=-objc IOLIBDIR=./iolib-objc PREFIX=$(SOLARIS_PREFIX) install

solaris-objc-static-uninstall:
	$(MAKE) -f makefiles/Makefile.posix-static SRCEXT=.m INCLUDEEXT=-objc IOLIBDIR=./iolib-objc PREFIX=$(SOLARIS_PREFIX) uninstall

solaris-objc-shared:
	$(MAKE) -f makefiles/Makefile.posix-shared SRCEXT=.m INCLUDEEXT=-objc IOLIBDIR=./iolib-objc OTHERLIBS="-lobjc -lsocket -lnsl -lstdc++ -lm"

solaris-objc-shared-clean:
	$(MAKE) -f makefiles/Makefile.posix-shared SRCEXT=.m INCLUDEEXT=-objc IOLIBDIR=./iolib-objc clean

solaris-objc-shared-install:
	$(MAKE) -f makefiles/Makefile.posix-shared SRCEXT=.m INCLUDEEXT=-objc IOLIBDIR=./iolib-objc PREFIX=$(SOLARIS_PREFIX) install

solaris-objc-shared-uninstall:
	$(MAKE) -f makefiles/Makefile.posix-shared SRCEXT=.m INCLUDEEXT=-objc IOLIBDIR=./iolib-objc PREFIX=$(SOLARIS_PREFIX) uninstall

win32-objc-static:
	$(MAKE) -f makefiles/Makefile.win32-static SRCEXT=.m INCLUDEEXT=-objc IOLIBDIR=./iolib-objc OTHERLIBS=-lobjc

win32-objc-static-clean:
	$(MAKE) -f makefiles/Makefile.win32-static SRCEXT=.m INCLUDEEXT=-objc IOLIBDIR=./iolib-objc clean

win32-objc-static-install:
	$(ECHO) "There is no standard install method for Windows targets!"
	$(ECHO) "Copy the files to a directory of your choice!"

win32-objc-static-uninstall:
	$(ECHO) "There is no standard uninstall method for Windows targets!"
	$(ECHO) "You have to remove the files manually!"
