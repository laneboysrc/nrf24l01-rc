.DEFAULT_GOAL := all


###############################################################################
# Configuration options for our application
TARGET := receiver
SOURCE_DIRS := .
BUILD_DIR := build

SYSTEM_CLOCK := 16000000

SOURCES := $(foreach sdir, $(SOURCE_DIRS), $(wildcard $(sdir)/*.c))
DEPENDENCIES := makefile platform.h nrf24le1.h
DEPENDENCIES += spi.h rc_receiver.h rf.h


###############################################################################
# Hardware selection
#
# Set the variable HARDWARE to one of the following:
#
# 	NRF24LE1_MODULE is our own hardware design using an off-the-shelf module
# 	HKR3000 is the blue HobbyKing receiver that ships with the HK310 transmitter
# 	XR3100 is the small HobbyKing receiver that ships with Turnigy 3XS
NRF24LE1_MODULE := 0
XR3100 := 1
HKR3000 := 2

HARDWARE := NRF24LE1_MODULE


###############################################################################
# Pretty-print setup
V ?= $(VERBOSE)
ifneq ($(V), 1)
QUIET := @
ECHO := @echo
else
QUIET :=
ECHO := @true
endif


###############################################################################
# Toolchain setup
TOOLCHAIN_PREFIX :=
TOOLCHAIN_PATH :=

CC := $(TOOLCHAIN_PATH)$(TOOLCHAIN_PREFIX)sdcc
LD := $(TOOLCHAIN_PATH)$(TOOLCHAIN_PREFIX)sdcc

MKDIR_P = mkdir -p
FLASH_TOOL := ../../nrf_prog_v1_0/nrf_spi_program_firmware.py
TERMINAL_PROGRAM := miniterm.py -p /dev/ttyUSB0 -b $(BAUDRATE) --echo



###############################################################################
# Target and object file setup
OBJECTS := $(patsubst %.c, $(BUILD_DIR)/%.rel, $(SOURCES))

TARGET_HEX := $(addprefix $(BUILD_DIR)/, $(TARGET).hex)
TARGET_BIN := $(addprefix $(BUILD_DIR)/, $(TARGET).bin)

$(OBJECTS): $(DEPENDENCIES)


###############################################################################
# Compiler and linker flags

CFLAGS := -mmcs51 --std-c99 -I.

CFLAGS += -D__SYSTEM_CLOCK=$(SYSTEM_CLOCK)
CFLAGS += -DNRF24LE1_MODULE=$(NRF24LE1_MODULE) -DXR3100=$(XR3100) -DHKR3000=$(HKR3000)
CFLAGS += -DHARDWARE=$(HARDWARE)
CFLAGS += -DNO_DEBUG
CFLAGS += -DENABLE_PREPROCESSOR_OUTPUT
#CFLAGS += -DEXTENDED_PREPROCESSOR_OUTPUT

LDFLAGS := --out-fmt-ihx
LDFLAGS += --code-size 0x4000 --xram-size 0x400

LDLIBS := $(addprefix -l,$(LIBS))


###############################################################################
# Plumbing for rules
vpath %.c $(SOURCE_DIRS)

dummy := $(shell $(MKDIR_P) $(BUILD_DIR))   # Always create the build directory

define compile-objects
$1/%.rel: %.c
	$(ECHO) [CC] $$<
	$(QUIET) $(CC) $(CFLAGS) -c $$< -o $$@
endef

$(foreach bdir, $(BUILD_DIR), $(eval $(call compile-objects,$(bdir))))


###############################################################################
# Rules
all : $(TARGET_BIN)

nrf24le1_module: HARDWARE := NRF24LE1_MODULE
xr3100: HARDWARE := XR3100
hkr3000: HARDWARE := HKR3000

nrf24le1_module: clean $(TARGET_HEX)
hkr3000: clean $(TARGET_HEX)
xr3100: clean $(TARGET_HEX)

$(TARGET_HEX): $(OBJECTS)
	$(ECHO) [INFO] Building for hardware $(HARDWARE)
	$(ECHO) [LD] $@
	$(QUIET) $(LD) $(LDFLAGS) -o $@ $(OBJECTS) $(LDLIBS)

$(TARGET_BIN): $(TARGET_HEX)
	$(ECHO) [HEX-\>BIN] $@
	$(QUIET) srec_cat -Disable_Sequence_Warnings $< -intel -o $@ -binary

# Invoke the tool to program the microcontroller
program: $(TARGET_BIN)
	$(QUIET) $(FLASH_TOOL) $<

# Invoke a tool for UART communication
terminal:
	$(QUIET) $(TERMINAL_PROGRAM)

# Clean all generated files
clean:
	$(ECHO) [RM] $(BUILD_DIR)
	$(QUIET) $(RM) -rf $(BUILD_DIR)/*


.PHONY : all clean program terminal xr3100 hkr3000 nrf24le1_module
