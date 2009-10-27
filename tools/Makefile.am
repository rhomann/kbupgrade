if CAN_BUILD_BOOTLOADHID
SUBDIRS=bootloadHID.2008-11-26/commandline
endif

AM_CPPFLAGS=-I$(top_srcdir)/firmware/common

if CAN_BUILD_KBUPTOOL
bin_PROGRAMS=kbuptool
kbuptool_SOURCES=kbuptool.c kbcom.c kbcom.h
kbuptool_LDADD=$(LIBUSB1LIBS)
endif

MATRIXDEF=$(top_srcdir)/matrices/M4-1.matrix

if CAN_BUILD_FIRMWARES
noinst_PROGRAMS=codectest
codectest_SOURCES=codectest.c

BUILT_SOURCES=files.stamp
endif

EXTRA_DIST=procmatrix.py\
 bootloadHID.2008-11-26/Changelog.txt\
 bootloadHID.2008-11-26/License.txt\
 bootloadHID.2008-11-26/Readme.txt

CLEANFILES=files.stamp keyboard.h stdstoredmap.h stdmap.h keymapdecoder.h

files.stamp: $(PROCMATRIX) $(MATRIXDEF)
	rm -f $@
	$(PROC) -k keyboard.h -h stdstoredmap.h -H stdmap.h -c keymapdecoder.h -N stored_map -P
	touch $@

keyboard.h: files.stamp

stdstoredmap.h: files.stamp

stdmap.h: files.stamp

keymapdecoder.h: files.stamp
