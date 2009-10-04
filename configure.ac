AC_PREREQ([2.63])
AC_INIT([Keyboard Upgrade],[0.1],[kbupgrade@gmx.net])
AM_INIT_AUTOMAKE([-Wall silent-rules foreign])
AC_CONFIG_SRCDIR([firmware/1391401/main.c])
AC_CONFIG_HEADERS([config.h])

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

USBVENDOR=`echo "Robert Homann $PACKAGE_BUGREPORT" | sed 's/\(.\)/'\''\1'\'',/g' | sed 's/,$//'`
USBVENDORLEN=`echo "Robert Homann $PACKAGE_BUGREPORT" | wc -c`
USBVENDORLEN=`expr $USBVENDORLEN - 1`
USBDEVICE=`echo "$PACKAGE_STRING" | sed 's/\(.\)/'\''\1'\'',/g' | sed 's/,$//'`
USBDEVICELEN=`echo "$PACKAGE_STRING" | wc -c`
USBDEVICELEN=`expr $USBDEVICELEN - 1`
AC_SUBST([USBVENDOR])
AC_SUBST([USBVENDORLEN])
AC_SUBST([USBDEVICE])
AC_SUBST([USBDEVICELEN])

# Checks for programs.
AC_PROG_AWK
AM_PROG_AS
AC_PROG_CC
AC_PROG_CC_C99

AS_IF([test x"$ac_cv_prog_cc_c99" = xno],
      [ AC_MSG_ERROR([No C99 compliant compiler found.]) ])

AC_ARG_VAR([PYTHON],[name of the Python interpreter])
AC_CHECK_PROGS([PYTHON],[python python2.6 python3 python3.0])
AS_IF([test -z "$PYTHON"],
      [ AC_MSG_ERROR([A Python installation is required to build the software (minimum version is 2.6). Please install and try again.]) ])

AC_CHECK_PROGS([AVRGCC],[avr-gcc])
AC_CHECK_PROGS([AVROBJCP],[avr-objcopy])
AS_IF([test -z "$AVRGCC" || test -z "$AVROBJCP"],
      [ AC_MSG_ERROR([Cross compiler for AVR not found or not installed correctly (package "gcc-avr" on Debian). Please install and try again.]) ])

AC_CHECK_PROGS([LIBUSBCFG],[libusb-config])
AC_ARG_VAR([AVRDUDE],[name of the avrdude executable])
AC_CHECK_PROGS([AVRDUDE],[avrdude])

AS_IF([test x"$AVRDUDE" != x],
      [ prog='$(PROGTOOL) -p $(AVRTYPE) -c $(AVRPROG)'
        PROGTOOL="$AVRDUDE"
        PROG="$prog"' -U flash:w:$<'
        PROGLOCK="$prog"' -U lock:w:$(LOCKFUSE):m'
        PROGFUSES="$prog"' -U lfuse:w:$(LOFUSE):m -U hfuse:w:$(HIFUSE):m'
        PROGREADFUSES="$prog"' -U lfuse:r:-:h -U hfuse:r:-:h -U lock:r:-:h 2> /dev/null' ],
      [ AC_MSG_WARN([Currently, only avrdude is supported for programming, but it was not found in the path. You may use the environment variable AVRDUDE to point at the binary. If you are using some other programming tool, you will have to use it manually for now.]) ])

AC_SUBST([PROGTOOL])
AC_SUBST([PROG])
AC_SUBST([PROGLOCK])
AC_SUBST([PROGFUSES])
AC_SUBST([PROGREADFUSES])

# Checks for libraries.
AS_IF([test x"${LIBUSBCFG}" = x],
  [
    AC_CHECK_LIB([usb],[usb_init],,
      [ AC_MSG_ERROR([Seems like libusb is not installed. Please install and try again (and don't forget the development files, too).])
      ])
    AC_CHECK_HEADER([usb.h],,
      [ AC_MSG_ERROR([Seems like the development files for libusb are not installed (called, e.g., "libusb-dev" on Debian). Please install and try again.])
      ])
  ],
  [ CFLAGS="${CFLAGS} "`libusb-config --cflags`
    LIBS="${LIBS} "`libusb-config --libs` ]
)

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

cat << END

Configured for Atmel $AVRTYPE at $AVRFREQ Hz.
Using $PROGTOOL for programming, type $AVRPROG.
END
