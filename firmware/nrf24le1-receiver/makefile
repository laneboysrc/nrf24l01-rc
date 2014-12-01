.DEFAULT_GOAL := all


###############################################################################
# Configuration options for our application
TARGET := receiver
SOURCE_DIRS := .
BUILD_DIR := build

SYSTEM_CLOCK := 16000000
BAUDRATE := 38400

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
OBJCOPY := $(TOOLCHAIN_PATH)$(TOOLCHAIN_PREFIX)objcopy

MKDIR_P = mkdir -p
FLASH_TOOL :=
TERMINAL_PROGRAM := miniterm.py -p /dev/ttyUSB0 -b $(BAUDRATE) --echo



###############################################################################
# Target and object file setup
OBJECTS := $(patsubst %.c, $(BUILD_DIR)/%.rel, $(SOURCES))

TARGET_HEX := $(addprefix $(BUILD_DIR)/, $(TARGET).hex)

$(OBJECTS): $(DEPENDENCIES)


###############################################################################
# Compiler and linker flags

CFLAGS := -mmcs51 --std-c99 -I.
CFLAGS += -D__SYSTEM_CLOCK=$(SYSTEM_CLOCK)
CFLAGS += --opt-code-speed
CFLAGS += -DNRF24LE1_MODULE=0 -DXR3100=1 -DHKR3000=2 -DHARDWARE=$(HARDWARE)
CFLAGS += -DBAUDRATE=$(BAUDRATE)
#CFLAGS += -DNO_DEBUG
#CFLAGS += -DENABLE_PREPROCESSOR_OUTPUT -DEXTENDED_PREPROCESSOR_OUTPUT

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
all : $(TARGET_HEX)

$(TARGET_HEX): $(OBJECTS)
	$(ECHO) [LD] $@
	$(QUIET) $(LD) $(LDFLAGS) -o $@ $(OBJECTS) $(LDLIBS)

# Invoke the tool to program the microcontroller
program: $(TARGET_BIN)
	$(QUIET )$(FLASH_TOOL) $<

# Invoke a tool for UART communication
terminal:
	$(QUIET) $(TERMINAL_PROGRAM)

# Clean all generated files
clean:
	$(ECHO) [RM] $(BUILD_DIR)
	$(QUIET) $(RM) -rf $(BUILD_DIR)/*


.PHONY : all clean program terminal
