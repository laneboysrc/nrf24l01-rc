# BINARY = stm32f1-nrf24l01-transmitter

# # OBJS = other_files.o

# LDSCRIPT = stm32f103c8t6.ld

# include libopencm3.target.mk



.DEFAULT_GOAL := all

PROJECT_NAME := tx


OPENCM3_DIR = ./libopencm3
OPENCM3_LIBNAME	= opencm3_stm32f1


# Supress or show diagnostic compiler output
ifeq ("$(VERBOSE)", "1")
ECHO :=
else
ifeq ("$(V)", "1")
ECHO :=
else
ECHO := @
endif
endif


# Toolchain commands
# GNU_INSTALL_ROOT 	:= /usr
GNU_INSTALL_ROOT 	:= /opt/gcc-arm-none-eabi-5_2-2015q4
GNU_PREFIX       	:= arm-none-eabi
CC               	:= '$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-gcc'
AS               	:= '$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-as'
AR               	:= '$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-ar' -r
LD               	:= '$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-ld'
NM               	:= '$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-nm'
OBJDUMP          	:= '$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-objdump'
OBJCOPY          	:= '$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-objcopy'
SIZE             	:= '$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-size'

OPENOCD          	:= /opt/openocd/bin/openocd

MK 				 	:= mkdir -p
RM 					:= rm -rf


# Source files, include paths, library paths and libraries
C_SOURCE_FILES += $(abspath main.c)

LIBS += -lc -lnosys
LIBS += -l$(OPENCM3_LIBNAME)

LINKER_SCRIPT = stm32f103c8t6.ld

INC_PATHS += -I$(abspath .)
INC_PATHS += -isystem$(abspath $(OPENCM3_DIR)/include)

LIB_PATHS += -L$(abspath $(OPENCM3_DIR)/lib)


# Build directory, where all the intermediate files go
OBJECT_DIRECTORY = build
LISTING_DIRECTORY = $(OBJECT_DIRECTORY)
OUTPUT_BINARY_DIRECTORY = $(OBJECT_DIRECTORY)

# Sorting removes duplicates
BUILD_DIRECTORIES := $(sort $(OBJECT_DIRECTORY) $(OUTPUT_BINARY_DIRECTORY) $(LISTING_DIRECTORY) )


# CPU architecture
ARCH_FLAGS = -DSTM32F1 -mcpu=cortex-m3 -mthumb -msoft-float -mfix-cortex-m3-ldrd


# C compiler flags
CFLAGS := $(ARCH_FLAGS)
CFLAGS += --std=c99
CFLAGS += -Wall -Werror -O3 -g3
# keep every function in separate section. This will allow linker to dump unused functions
CFLAGS += -fno-common -ffunction-sections -fdata-sections -fno-strict-aliasing
CFLAGS += -fno-builtin --short-enums


# Linker flags
LDFLAGS := $(ARCH_FLAGS)
LDFLAGS	+= --static
LDFLAGS	+= -nostartfiles
LDFLAGS += $(LIB_PATHS)
LDFLAGS += -T$(LINKER_SCRIPT)
LDFLAGS += -Wl,-Map=$(LISTING_DIRECTORY)/$(PROJECT_NAME).map
LDFLAGS += -Wl,--gc-sections
LDFLAGS += --specs=nano.specs


# Create a list of object files and paths
C_SOURCE_FILE_NAMES = $(notdir $(C_SOURCE_FILES))
C_PATHS = $(sort $(dir $(C_SOURCE_FILES) ) )
C_OBJECTS = $(addprefix $(OBJECT_DIRECTORY)/, $(C_SOURCE_FILE_NAMES:.c=.o) )

vpath %.c $(C_PATHS)

OBJECTS = $(C_OBJECTS)


# Building all targets
all: $(OUTPUT_BINARY_DIRECTORY)/$(PROJECT_NAME).hex echosize

# Create build directories
$(BUILD_DIRECTORIES):
	@echo [MKDIR] $@
	$(ECHO)$(MK) $@

# Create objects from C source files
$(OBJECT_DIRECTORY)/%.o: %.c
	@echo [CC] $(notdir $<)
	$(ECHO)$(CC) $(CFLAGS) $(INC_PATHS) -c -o $@ $<

# Link
$(OUTPUT_BINARY_DIRECTORY)/$(PROJECT_NAME).out: $(BUILD_DIRECTORIES) $(OBJECTS)
	@echo [LD] $(PROJECT_NAME).out
	$(ECHO)$(CC) $(LDFLAGS) $(OBJECTS) $(LIBS) -o $(OUTPUT_BINARY_DIRECTORY)/$(PROJECT_NAME).out

# Create binary .bin file from the .out file
$(OUTPUT_BINARY_DIRECTORY)/$(PROJECT_NAME).bin: $(OUTPUT_BINARY_DIRECTORY)/$(PROJECT_NAME).out
	@echo [OBJCOPY] $(PROJECT_NAME).bin
	$(ECHO)$(OBJCOPY) -O binary $(OUTPUT_BINARY_DIRECTORY)/$(PROJECT_NAME).out $(OUTPUT_BINARY_DIRECTORY)/$(PROJECT_NAME).bin

# Create binary .hex file from the .out file
$(OUTPUT_BINARY_DIRECTORY)/$(PROJECT_NAME).hex: $(OUTPUT_BINARY_DIRECTORY)/$(PROJECT_NAME).out
	@echo [OBJCOPY] $(PROJECT_NAME).hex
	$(ECHO)$(OBJCOPY) -O ihex $(OUTPUT_BINARY_DIRECTORY)/$(PROJECT_NAME).out $(OUTPUT_BINARY_DIRECTORY)/$(PROJECT_NAME).hex

echosize:
	-@echo ''
	$(ECHO)$(SIZE) $(OUTPUT_BINARY_DIRECTORY)/$(PROJECT_NAME).out
	-@echo ''

clean:
	$(ECHO)$(RM) $(BUILD_DIRECTORIES)

program: $(OUTPUT_BINARY_DIRECTORY)/$(PROJECT_NAME).hex
	$(OPENOCD) -f interface/stlink-v2.cfg -f target/nrf51.cfg -c "init; reset halt ; nrf51 mass_erase ; sleep 500 ; flash write_image $(OUTPUT_BINARY_DIRECTORY)/$(PROJECT_NAME).hex ; reset ; shutdown"

flash: program

reset:
	$(OPENOCD) -f interface/stlink-v2.cfg -f target/nrf51.cfg -c "init; reset ; shutdown"

terminal:
	miniterm.py -p /dev/ttyUSB0 -b 460800

