.DEFAULT_GOAL := all

PROJECT_NAME := tx

###############################################################################
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


###############################################################################
# Toolchain commands
GNU_INSTALL_ROOT 	:= /opt/gcc-arm-none-eabi-5_2-2015q4
GNU_PREFIX       	:= arm-none-eabi
CC               	:= '$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-gcc'
AS               	:= '$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-as'
AR               	:= '$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-ar' -r
LD               	:= '$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-ld'
NM               	:= '$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-nm'
GDB               	:= '$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-gdb'
OBJDUMP          	:= '$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-objdump'
OBJCOPY          	:= '$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-objcopy'
SIZE             	:= '$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-size'

OPENOCD          	:= /opt/openocd/bin/openocd
OPENOCD_INTERFACE   := stlink-v2
OPENOCD_TARGET      := stm32f1x

MK 				 	:= mkdir -p
RM 					:= rm -rf

OPENCM3_DIR 		:= ./libs/libopencm3
OPENCM3_LIBNAME		:= opencm3_stm32f1


###############################################################################
# Source files, include paths, library paths and libraries
C_SOURCE_FILES += $(abspath curves.c)
C_SOURCE_FILES += $(abspath main.c)
C_SOURCE_FILES += $(abspath mixer.c)
C_SOURCE_FILES += $(abspath sound/music.c)
C_SOURCE_FILES += $(abspath nrf24l01p.c)
C_SOURCE_FILES += $(abspath protocol_hk310.c)
C_SOURCE_FILES += $(abspath ring_buffer.c)
C_SOURCE_FILES += $(abspath sound/sound.c)
C_SOURCE_FILES += $(abspath spi.c)
C_SOURCE_FILES += $(abspath systick.c)
C_SOURCE_FILES += $(abspath uart.c)

LIBS += -lc -lnosys
LIBS += -l$(OPENCM3_LIBNAME)

LINKER_SCRIPT = stm32f103c8t6.ld

INC_PATHS += -I$(abspath .)
INC_PATHS += -I$(abspath ./sound)
INC_PATHS += -isystem$(abspath $(OPENCM3_DIR)/include)

LIB_PATHS += -L$(abspath $(OPENCM3_DIR)/lib)


###############################################################################
# Build directory, where all the intermediate files go
OBJECT_DIRECTORY = _build
LISTING_DIRECTORY = $(OBJECT_DIRECTORY)
OUTPUT_BINARY_DIRECTORY = $(OBJECT_DIRECTORY)

# Sorting removes duplicates
BUILD_DIRECTORIES := $(sort $(OBJECT_DIRECTORY) $(OUTPUT_BINARY_DIRECTORY) $(LISTING_DIRECTORY) )


###############################################################################
# CPU architecture
ARCH_FLAGS = -DSTM32F1 -mcpu=cortex-m3 -mthumb -msoft-float -mfix-cortex-m3-ldrd


###############################################################################
# C compiler flags
CFLAGS := $(ARCH_FLAGS)
CFLAGS += --std=c99
CFLAGS += -ggdb3
CFLAGS += -MMD -MP
CFLAGS += -W -Wall -Wextra -Wpedantic
# Be even more pedantic ...
CFLAGS += -Waddress
CFLAGS += -Wbool-compare
CFLAGS += -Wdeclaration-after-statement
CFLAGS += -Wdouble-promotion
CFLAGS += -Wfloat-equal
CFLAGS += -Wlogical-op
CFLAGS += -Wmissing-declarations
CFLAGS += -Wmissing-field-initializers
CFLAGS += -Wmissing-prototypes
CFLAGS += -Wold-style-definition
CFLAGS += -Wpacked
CFLAGS += -Wparentheses
CFLAGS += -Wshadow
CFLAGS += -Wstrict-prototypes
CFLAGS += -Wswitch-default
CFLAGS += -Wswitch-enum
CFLAGS += -Wundef
CFLAGS += -Wwrite-strings
# Keep every function in separate section. This will allow linker to dump unused functions
CFLAGS += -ffunction-sections -fdata-sections
CFLAGS += -fno-builtin
CFLAGS += -fsigned-char
CFLAGS += -Os

###############################################################################
# Linker flags
LDFLAGS := $(ARCH_FLAGS)
LDFLAGS	+= --static
LDFLAGS	+= -nostartfiles
LDFLAGS += $(LIB_PATHS)
LDFLAGS += -T$(LINKER_SCRIPT)
LDFLAGS += -Wl,-Map=$(LISTING_DIRECTORY)/$(PROJECT_NAME).map
LDFLAGS += -Wl,--gc-sections
LDFLAGS += --specs=nano.specs


###############################################################################
# Create a list of object files and paths
C_SOURCE_FILE_NAMES = $(notdir $(C_SOURCE_FILES))
C_PATHS = $(sort $(dir $(C_SOURCE_FILES) ) )
C_OBJECTS = $(addprefix $(OBJECT_DIRECTORY)/, $(C_SOURCE_FILE_NAMES:.c=.o) )

vpath %.c $(C_PATHS)

OBJECTS = $(C_OBJECTS)


###############################################################################
libopencm3 :
	$(ECHO)if [ ! "`ls -A $(OPENCM3_DIR)/lib`" ] ; then \
		printf "######## ERROR ########\n"; \
		printf "libopencm3 is not initialized.\n"; \
		printf "Please run:\n"; \
		printf "\n"; \
		printf "  $$ cd ../..\n"; \
		printf "  $$ git submodule init\n"; \
		printf "  $$ git submodule update\n"; \
		printf "\n"; \
		printf "before running make.\n"; \
		printf "\n"; \
		exit 1;\
		fi
	$(ECHO)if [ ! "`ls $(OPENCM3_DIR)/lib/lib$(OPENCM3_LIBNAME).a`" ] ; then \
		$(MAKE) -C $(OPENCM3_DIR) TARGETS=stm32/f1 ; \
		fi


###############################################################################
# Build rules

# Building all targets
all : $(OUTPUT_BINARY_DIRECTORY)/$(PROJECT_NAME).hex echosize

# Create build directories
$(BUILD_DIRECTORIES) : libopencm3
	@echo [MKDIR] $@
	$(ECHO)$(MK) $@

# Create objects from C source files
$(OBJECT_DIRECTORY)/%.o : %.c
	@echo [CC] $(notdir $<)
	$(ECHO)$(CC) $(CFLAGS) $(INC_PATHS) -c -o $@ $<

# Create list files
$(LISTING_DIRECTORY)/%.lst : %.c
	@echo [LIST] $<
	$(ECHO)$(CC) $(CFLAGS) $(INC_PATHS) -c -g -Wa,-adlhn -o $(patsubst %.lst,%.o,$@) $< > $@

# Link
$(OUTPUT_BINARY_DIRECTORY)/$(PROJECT_NAME).out : $(BUILD_DIRECTORIES) $(OBJECTS)
	@echo [LD] $(PROJECT_NAME).out
	$(ECHO)$(CC) $(LDFLAGS) $(OBJECTS) $(LIBS) -o $(OUTPUT_BINARY_DIRECTORY)/$(PROJECT_NAME).out

# Create binary .bin file from the .out file
$(OUTPUT_BINARY_DIRECTORY)/$(PROJECT_NAME).bin : $(OUTPUT_BINARY_DIRECTORY)/$(PROJECT_NAME).out
	@echo [OBJCOPY] $(PROJECT_NAME).bin
	$(ECHO)$(OBJCOPY) -O binary $(OUTPUT_BINARY_DIRECTORY)/$(PROJECT_NAME).out $(OUTPUT_BINARY_DIRECTORY)/$(PROJECT_NAME).bin

# Create binary .hex file from the .out file
$(OUTPUT_BINARY_DIRECTORY)/$(PROJECT_NAME).hex : $(OUTPUT_BINARY_DIRECTORY)/$(PROJECT_NAME).out
	@echo [OBJCOPY] $(PROJECT_NAME).hex
	$(ECHO)$(OBJCOPY) -O ihex $(OUTPUT_BINARY_DIRECTORY)/$(PROJECT_NAME).out $(OUTPUT_BINARY_DIRECTORY)/$(PROJECT_NAME).hex

# Create list files that include C code as well as Assembler
list : $(BUILD_DIRECTORIES) $(OBJECTS:.o=.lst)

echosize :
	-@echo ''
	$(ECHO)$(SIZE) $(OUTPUT_BINARY_DIRECTORY)/$(PROJECT_NAME).out
	-@echo ''

clean :
	$(ECHO)$(RM) $(BUILD_DIRECTORIES)

program : $(OUTPUT_BINARY_DIRECTORY)/$(PROJECT_NAME).hex
	$(OPENOCD) -f interface/$(OPENOCD_INTERFACE).cfg -f target/$(OPENOCD_TARGET).cfg -c "init ; reset init ; flash write_image erase $(OUTPUT_BINARY_DIRECTORY)/$(PROJECT_NAME).hex ; reset ; shutdown"

flash : program

reset :
	$(OPENOCD) -f interface/$(OPENOCD_INTERFACE).cfg -f target/$(OPENOCD_TARGET).cfg -c "init; reset ; shutdown"

# Invoke openocd for a debugging session
debug : $(OUTPUT_BINARY_DIRECTORY)/$(PROJECT_NAME).out
	$(ECHO)$(GDB) -ex "target extended-remote | $(OPENOCD) -f interface/$(OPENOCD_INTERFACE).cfg -f target/$(OPENOCD_TARGET).cfg -c \"gdb_port pipe; log_output openocd.log\"" $<

terminal :
	miniterm.py -p /dev/ttyUSB0 -b 115200


.PHONY : clean program flash reset terminal echosize list all debug libopencm3


###############################################################################
# Include (optional) dependency files. Needs -MD in CFLAGS
-include $(OBJECTS:.o=.d)
