AC_PREREQ([2.63])
AC_INIT([Keyboard Upgrade],[0.1.0],[kbupgrade@gmx.net])
AM_INIT_AUTOMAKE([-Wall silent-rules foreign])
AC_CONFIG_SRCDIR([firmware/1391401/main.c])
AC_CONFIG_HEADERS([config.h])

ku_warnings=''
AC_DEFUN([KU_MSG_WARN],[
  AC_MSG_WARN([$1])
  ku_warnings="$ku_warnings- AS_ESCAPE([$1])\n"
])

AC_ARG_WITH([avr-type],
  [AS_HELP_STRING([--with-avr-type],
                  [compile firmware for selected AVR (atmega16 (default), atmega32)]) ],
  [ AVRTYPE="${withval}" ],
  [ AVRTYPE=atmega16 ]
)

AC_ARG_WITH([avr-freq],
  [AS_HELP_STRING([--with-avr-freq],
                  [select the AVR clock frequency in Hz (default: 12000000)]) ],
  [ AVRFREQ="${withval}L" ],
  [ AVRFREQ=12000000L ]
)

AC_ARG_WITH([avrdude-programmer],
  [AS_HELP_STRING([--with-avrdude-programmer],
                  [select AVR programmer type when using avrdude for AVR programming (default: usbasp)]) ],
  [ AVRPROG="${withval}" ],
  [ AVRPROG=usbasp ]
)

# For ATMEGA16 and ATMEGA32:
#
#   fuse byte low:
#     0    brown-out detector triggers at 4 V
#     0    brown-out detector enabled
#     01   crystal oscillator with brown-out detector
#     111  3-8 MHz (high speed mode)
#     1    crystal oscillator
#
#   fuse byte high:
#     1    disable OCD
#     1    disable JTAG
#     0    enable SPI
#     1    small oscillator swing
#     0    preserve EEPROM
#     0x   boot area size 1024 words, 2 kB (00 on ATMEGA16, 01 on ATMEGA32)
#     0    reset vector to boot loader area
#
#   lock fuse:
#     00   reserved
#     10   SPM is not allowed to write to boot section
#     11   no restriction on application section
#     11   no memory lock features
AS_CASE([$AVRTYPE],
  [atmega16],
  [ BOOTLOADER_ADDRESS=0x3800
    LOFUSE=0x1f
    HIFUSE=0xd0
    LOCKFUSE=0x2f
  ],
  [atmega32],
  [ BOOTLOADER_ADDRESS=0x7800
    LOFUSE=0x1f
    HIFUSE=0xd2
    LOCKFUSE=0x2f
  ],
  [ AC_MSG_ERROR([Unsupported AVR type: $AVRTYPE]) ]
)

AC_SUBST([AVRTYPE])
AC_SUBST([AVRFREQ])
AC_SUBST([AVRPROG])
AC_SUBST([BOOTLOADER_ADDRESS])
AC_SUBST([LOFUSE])
AC_SUBST([HIFUSE])
AC_SUBST([LOCKFUSE])

USBVENDORSTR='Robert Homann '"$PACKAGE_BUGREPORT"
USBDEVICESTR="$PACKAGE_STRING"

USBVENDOR=`echo "$USBVENDORSTR" | sed 's/\(.\)/'\''\1'\'',/g' | sed 's/,$//'`
USBVENDORLEN=`echo "USBVENDORSTR" | wc -c`
USBVENDORLEN=`expr $USBVENDORLEN - 1`
USBDEVICE=`echo "$USBDEVICESTR" | sed 's/\(.\)/'\''\1'\'',/g' | sed 's/,$//'`
USBDEVICELEN=`echo "$USBDEVICESTR" | wc -c`
USBDEVICELEN=`expr $USBDEVICELEN - 1`
AC_SUBST([USBVENDOR])
AC_SUBST([USBVENDORLEN])
AC_SUBST([USBDEVICE])
AC_SUBST([USBDEVICELEN])

# 0x16c0 = 5824 = voti.nl
USBVENDORID='0xc0, 0x16'
# 0x05dc = 1500
USBDEVICEID='0xdc, 0x05'
AC_SUBST([USBVENDORID])
AC_SUBST([USBDEVICEID])
AC_DEFINE([KBUPGRADE_VENDOR_ID],[((uint16_t)0x16c0)],[Vendor ID of keyboards equipped with Keyboard Upgrade.])
AC_DEFINE([KBUPGRADE_DEVICE_ID],[((uint16_t)0x05dc)],[Device ID of keyboards equipped with Keyboard Upgrade.])
AC_DEFINE_UNQUOTED([KBUPGRADE_VENDOR_NAME],["$USBVENDORSTR"],[Vendor name of keyboards equipped with Keyboard Upgrade.])
AC_DEFINE_UNQUOTED([KBUPGRADE_DEVICE_NAME],["$USBDEVICESTR"],[Device name of keyboards equipped with Keyboard Upgrade.])

build_firmwares=yes
build_bootloadhid=yes
build_kbuptool=yes

# Checks for programs.
AC_PROG_AWK
AM_PROG_AS
AC_PROG_CC
AC_PROG_CC_C99

AS_IF([test x"$ac_cv_prog_cc_c99" = xno],
      [ AC_MSG_ERROR([No C99 compliant compiler found.]) ])

AC_ARG_VAR([PYTHON],[name of the Python interpreter])
AC_CHECK_PROGS([PYTHON],[python python2.6 python3 python3.0])
AS_IF([test x"$PYTHON" = x],
      [ KU_MSG_WARN([A Python installation is required to build the firmware, but none was found (minimum version is 2.6). If the interpreter is not in your PATH, then set the PYTHON environment variable.]) ],
      [ AS_VERSION_COMPARE([`$PYTHON -V 2>&1 | cut -d ' ' -f 2-`],[2.6],
          [ KU_MSG_WARN([Your Python installation is too old, minimum required version is 2.6 for building the firmware. You may try pointing at the correct Python interpreter using the PYTHON environment variable.])
            PYTHON='' ])
      ])
AS_IF([test x"$PYTHON" = x],
      [ PYTHON=false
        build_firmwares=no
      ])

AC_ARG_VAR([AVRGCC],[name of the gcc for AVR cross compiler executable])
AC_ARG_VAR([AVROBJCP],[name of the objcopy executable for AVR])
AC_CHECK_PROGS([AVRGCC],[avr-gcc])
AC_CHECK_PROGS([AVROBJCP],[avr-objcopy])
AS_IF([test x"$AVRGCC" = x || test x"$AVROBJCP" = x],
      [ KU_MSG_WARN([gcc for AVR not found or not installed correctly (package "gcc-avr" on Debian). Please install and try again if you would like to be able to build the firmware images.])
        AVRGCC=false
        AVROBJCP=false
        build_firmwares=no
      ])

AC_CHECK_PROGS([LIBUSBCFG],[libusb-config])
AC_ARG_VAR([AVRDUDE],[name of the avrdude executable])
AC_CHECK_PROGS([AVRDUDE],[avrdude])

AS_IF([test x"$AVRDUDE" = x],
      [ dnl Still set to avrdude just to get a useful command line
        dnl for users of other tools.
        AVRDUDE=avrdude
        KU_MSG_WARN([Currently, only avrdude is supported for programming, but it was not found in your PATH. You may use the environment variable AVRDUDE to point at the binary. If you are using some other programming tool, you will have to use it manually for now. Try, e.g., "make && make -n fuses flashboot flash lock" inside some firmware subdirectory to get a clue on the command line to use (take a look at the last four lines).])
      ])
prog='$(PROGTOOL) -p $(AVRTYPE) -c $(AVRPROG)'
PROGTOOL="$AVRDUDE"
PROG="$prog"' -U flash:w:$<'
PROGLOCK="$prog"' -U lock:w:$(LOCKFUSE):m'
PROGFUSES="$prog"' -U lfuse:w:$(LOFUSE):m -U hfuse:w:$(HIFUSE):m'
PROGREADFUSES="$prog"' -U lfuse:r:-:h -U hfuse:r:-:h -U lock:r:-:h 2> /dev/null'

AC_SUBST([PROGTOOL])
AC_SUBST([PROG])
AC_SUBST([PROGLOCK])
AC_SUBST([PROGFUSES])
AC_SUBST([PROGREADFUSES])

# Checks for libraries.
AS_IF([test x"$LIBUSBCFG" = x],
  [
    save_LIBS="$LIBS"
    AC_CHECK_LIB([usb],[usb_find_busses],,
      [ KU_MSG_WARN([Seems like libusb 0.1.x is not installed. Please install and try again if you would like to be able to build the boot loader command line tool (and don't forget the development files, too).])
        build_bootloadhid=no
      ])
    AC_CHECK_HEADER([usb.h],,
      [ KU_MSG_WARN([Seems like the development files for libusb 0.1.x are not installed (called, e.g., "libusb-dev" on Debian). Please install and try again if you would like to be able to build the boot loader command line tool.])
        build_bootloadhid=no
      ])
    AS_IF([test $build_bootloadhid = yes],
          [ LIBUSB0CFLAGS=''
            LIBUSB0LIBS='-lusb' ])
    LIBS="$save_LIBS"
  ],
  [ LIBUSB0CFLAGS="`libusb-config --cflags`"
    LIBUSB0LIBS="`libusb-config --libs`" ]
)

save_LIBS="$LIBS"
AC_CHECK_LIB([usb-1.0],[libusb_init],,
  [ KU_MSG_WARN([Seems like libusb 1.0.x is not installed. Please install and try again if you would like to be able to build the Keyboard Upgrade command line tool (and don't forget the development files, too).])
    build_kbuptool=no
  ])
AC_CHECK_HEADER([libusb-1.0/libusb.h],,
  [ KU_MSG_WARN([Seems like the development files for libusb 1.0.x are not installed (called, e.g., "libusb-1.0.0-dev" on Debian). Please install and try again if you would like to be able to build the Keyboard Upgrade command line tool.])
    build_kbuptool=no
  ])
AS_IF([test $build_kbuptool = yes],[ LIBUSB1LIBS='-lusb-1.0' ])
LIBS="$save_LIBS"

AC_SUBST([LIBUSB0CFLAGS])
AC_SUBST([LIBUSB0LIBS])
AC_SUBST([LIBUSB1LIBS])

# Checks for header files.
AC_CHECK_HEADERS([inttypes.h stdlib.h string.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([memset strerror strtol])

KEYCODES='$(top_srcdir)/firmware/common/usbkeycodes.h'
PROCMATRIX='$(top_srcdir)/tools/procmatrix.py'
PROC='$(PYTHON) $(PROCMATRIX) -d $(MATRIXDEF) -U $(KEYCODES)'
AC_SUBST([KEYCODES])
AC_SUBST([PROCMATRIX])
AC_SUBST([PROC])

AM_CONDITIONAL([CAN_BUILD_FIRMWARES],[test $build_firmwares = yes])
AM_CONDITIONAL([CAN_BUILD_BOOTLOADHID],[test $build_bootloadhid = yes])
AM_CONDITIONAL([CAN_BUILD_KBUPTOOL],[test $build_kbuptool = yes])

AC_CONFIG_FILES([Makefile
                 firmware/1391401/Makefile
                 firmware/1392934/Makefile
                 firmware/M4-1/Makefile
                 firmware/common/Makefile
                 firmware/common/usbconfig.h
                 firmware/vusb/Makefile
                 mappings/Makefile
                 matrices/Makefile
                 tools/Makefile
                 tools/bootloadHID.2008-11-26/commandline/Makefile])
AC_OUTPUT

AS_IF([ test -n "$ku_warnings" ],
      [ printf '\n***\n*** One or more problems were detected during configuration.\n*** This may mean that the firmware images cannot be built properly.\n***\nCollected warnings:\n'"$ku_warnings" ])

cat <<END

Configured for Atmel $AVRTYPE at $AVRFREQ Hz.
Using $PROGTOOL for programming, type $AVRPROG.
END
