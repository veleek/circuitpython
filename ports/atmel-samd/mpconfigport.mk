# All linking can be done with this common templated linker script, which has
# parameters that vary based on chip and/or board.
LD_TEMPLATE_FILE = boards/common.template.ld

INTERNAL_LIBM = 1

# Number of USB endpoint pairs.
USB_NUM_ENDPOINT_PAIRS = 8

CIRCUITPY_ROTARYIO_SOFTENCODER = 1
CIRCUITPY_OPTIMIZE_PROPERTY_FLASH_SIZE ?= 1
CIRCUITPY_LTO = 1

CIRCUITPY_KEYPAD_DEMUX ?= 0

######################################################################
# Put samd21-only choices here.

ifeq ($(CHIP_FAMILY),samd21)

# The ?='s allow overriding in mpconfigboard.mk.

# Some of these are on by default with CIRCUITPY_FULL_BUILD, but don't
# fit in 256kB of flash

CIRCUITPY_AESIO ?= 0
CIRCUITPY_ATEXIT ?= 0
CIRCUITPY_AUDIOMIXER ?= 0
CIRCUITPY_AUDIOMP3 ?= 0
CIRCUITPY_BINASCII ?= 0
CIRCUITPY_BITBANGIO ?= 0
CIRCUITPY_BITMAPFILTER ?= 0
CIRCUITPY_BITMAPTOOLS ?= 0
CIRCUITPY_BLEIO_HCI = 0
CIRCUITPY_BUILTINS_POW3 ?= 0
CIRCUITPY_BUSDEVICE ?= 0
CIRCUITPY_COMPUTED_GOTO_SAVE_SPACE ?= 1
CIRCUITPY_COUNTIO ?= 0
# Not enough RAM for framebuffers
CIRCUITPY_FRAMEBUFFERIO ?= 0
CIRCUITPY_FREQUENCYIO ?= 0
CIRCUITPY_GETPASS ?= 0
CIRCUITPY_GIFIO ?= 0
CIRCUITPY_I2CTARGET ?= 0
CIRCUITPY_JSON ?= 0
CIRCUITPY_KEYPAD ?= 0
CIRCUITPY_MSGPACK ?= 0
CIRCUITPY_OS_GETENV ?= 0
CIRCUITPY_PIXELMAP ?= 0
CIRCUITPY_RE ?= 0
CIRCUITPY_SDCARDIO ?= 0
CIRCUITPY_SPITARGET ?= 0
CIRCUITPY_SYNTHIO ?= 0
CIRCUITPY_TOUCHIO_USE_NATIVE ?= 1
CIRCUITPY_TRACEBACK ?= 0
CIRCUITPY_ULAB = 0
CIRCUITPY_VECTORIO = 0
CIRCUITPY_ZLIB = 0

# Turn off a few more things that don't fit in 192kB

CIRCUITPY_TERMINALIO_VT100 = 0

ifeq ($(INTERNAL_FLASH_FILESYSTEM),1)
CIRCUITPY_ONEWIREIO ?= 0
CIRCUITPY_SAFEMODE_PY ?= 0
CIRCUITPY_USB_IDENTIFICATION ?= 0
endif

MICROPY_PY_ASYNC_AWAIT ?= 0

# We don't have room for the fonts for terminalio for certain languages,
# so turn off terminalio, and if it's off and displayio is on,
# force a clean build.
# Note that we cannot test $(CIRCUITPY_DISPLAYIO) directly with an
# ifeq, because it's not set yet.
ifneq (,$(filter $(TRANSLATION),ja ko ru))
CIRCUITPY_TERMINALIO = 0
RELEASE_NEEDS_CLEAN_BUILD = $(CIRCUITPY_DISPLAYIO)
endif

SUPEROPT_GC = 0
SUPEROPT_VM = 0

CIRCUITPY_LTO_PARTITION = one

# On smaller builds this saves about 180 bytes. On other boards, it may -increase- space used, so use with care.
CFLAGS_BOARD = -fweb -frename-registers

endif # samd21
######################################################################

######################################################################
# Put samd51-only choices here.

ifeq ($(CHIP_FAMILY),samd51)

# No native touchio on SAMD51.
CIRCUITPY_TOUCHIO_USE_NATIVE = 0

ifeq ($(CIRCUITPY_FULL_BUILD),0)
CIRCUITPY_LTO_PARTITION ?= one
endif

# 20A skus have 1MB flash and can fit more.
ifeq ($(patsubst %20A,,$(CHIP_VARIANT)),)
HAS_1MB_FLASH = 1
else
HAS_1MB_FLASH = 0
endif

# The ?='s allow overriding in mpconfigboard.mk.


CIRCUITPY_ALARM ?= 1
# Not enough room for both bitmaptools and bitmapfilter on most boards.
CIRCUITPY_BITMAPFILTER ?= 0
CIRCUITPY_FLOPPYIO ?= $(CIRCUITPY_FULL_BUILD)
CIRCUITPY_FRAMEBUFFERIO ?= $(CIRCUITPY_FULL_BUILD)
CIRCUITPY_MAX3421E ?= $(HAS_1MB_FLASH)
CIRCUITPY_PS2IO ?= 1
CIRCUITPY_RGBMATRIX ?= $(CIRCUITPY_FRAMEBUFFERIO)
CIRCUITPY_SAMD ?= 1
CIRCUITPY_SPITARGET ?= 1
CIRCUITPY_SYNTHIO_MAX_CHANNELS = 12
CIRCUITPY_ULAB_OPTIMIZE_SIZE ?= 1
CIRCUITPY_WATCHDOG ?= 1

ifeq ($(CHIP_VARIANT),SAMD51G19A)
# No I2S on SAMD51G
CIRCUITPY_AUDIOBUSIO = 0
endif

endif # samd51
######################################################################

######################################################################
# Put same51-only choices here.

ifeq ($(CHIP_FAMILY),same51)

# No native touchio on SAME51.
CIRCUITPY_TOUCHIO_USE_NATIVE = 0

ifeq ($(CIRCUITPY_FULL_BUILD),0)
CIRCUITPY_LTO_PARTITION ?= one
endif

# The ?='s allow overriding in mpconfigboard.mk.

CIRCUITPY_ALARM ?= 1
CIRCUITPY_PS2IO ?= 1
CIRCUITPY_SAMD ?= 1
CIRCUITPY_FLOPPYIO ?= $(CIRCUITPY_FULL_BUILD)
CIRCUITPY_FRAMEBUFFERIO ?= $(CIRCUITPY_FULL_BUILD)
CIRCUITPY_RGBMATRIX ?= $(CIRCUITPY_FRAMEBUFFERIO)
CIRCUITPY_ULAB_OPTIMIZE_SIZE ?= 1

endif # same51
######################################################################

CIRCUITPY_BUILD_EXTENSIONS ?= uf2
