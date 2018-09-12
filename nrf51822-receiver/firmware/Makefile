.DEFAULT_GOAL := all

PROJECT_NAME := nrf51822-receiver

NRF_SDK_PATH = nrf51_sdk


#echo suspend
ifeq ("$(VERBOSE)", "1")
ECHO :=
else
ECHO := @
endif

# Toolchain commands
TEMPLATE_PATH    	:= $(NRF_SDK_PATH)/components/toolchain/gcc
GNU_INSTALL_ROOT 	?=
GNU_PREFIX       	:= arm-none-eabi
CC               	:= '$(GNU_INSTALL_ROOT)$(GNU_PREFIX)-gcc'
AS               	:= '$(GNU_INSTALL_ROOT)$(GNU_PREFIX)-as'
AR               	:= '$(GNU_INSTALL_ROOT)$(GNU_PREFIX)-ar' -r
LD               	:= '$(GNU_INSTALL_ROOT)$(GNU_PREFIX)-ld'
NM               	:= '$(GNU_INSTALL_ROOT)$(GNU_PREFIX)-nm'
OBJDUMP          	:= '$(GNU_INSTALL_ROOT)$(GNU_PREFIX)-objdump'
OBJCOPY          	:= '$(GNU_INSTALL_ROOT)$(GNU_PREFIX)-objcopy'
GDB             	:= '$(GNU_INSTALL_ROOT)$(GNU_PREFIX)-gdb'
SIZE             	:= '$(GNU_INSTALL_ROOT)$(GNU_PREFIX)-size'

MK 				 	:= mkdir -p
RM 					:= rm -rf


C_SOURCE_FILES  = $(abspath main.c)
C_SOURCE_FILES += $(abspath persistent_storage.c)
C_SOURCE_FILES += $(abspath rc_receiver.c)
C_SOURCE_FILES += $(abspath servo.c)
C_SOURCE_FILES += $(abspath vendor/nrf51-micro-esb/common/micro_esb.c)
C_SOURCE_FILES += $(abspath $(NRF_SDK_PATH)/components/drivers_nrf/clock/nrf_drv_clock.c)
C_SOURCE_FILES += $(abspath $(NRF_SDK_PATH)/components/drivers_nrf/common/nrf_drv_common.c)
C_SOURCE_FILES += $(abspath $(NRF_SDK_PATH)/components/drivers_nrf/ppi/nrf_drv_ppi.c)
C_SOURCE_FILES += $(abspath $(NRF_SDK_PATH)/components/drivers_nrf/rtc/nrf_drv_rtc.c)
C_SOURCE_FILES += $(abspath $(NRF_SDK_PATH)/components/drivers_nrf/timer/nrf_drv_timer.c)
C_SOURCE_FILES += $(abspath $(NRF_SDK_PATH)/components/drivers_nrf/uart/nrf_drv_uart.c)
C_SOURCE_FILES += $(abspath $(NRF_SDK_PATH)/components/libraries/fifo/app_fifo.c)
C_SOURCE_FILES += $(abspath $(NRF_SDK_PATH)/components/libraries/uart/app_uart_fifo.c)
C_SOURCE_FILES += $(abspath $(NRF_SDK_PATH)/components/libraries/uart/retarget.c)
C_SOURCE_FILES += $(abspath $(NRF_SDK_PATH)/components/libraries/util/app_util_platform.c)
C_SOURCE_FILES += $(abspath $(NRF_SDK_PATH)/components/toolchain/system_nrf51.c)

ASM_SOURCE_FILES = $(abspath $(NRF_SDK_PATH)/components/toolchain/gcc/gcc_startup_nrf51.s)

INC_PATHS += -I$(abspath .)
INC_PATHS += -I$(abspath $(NRF_SDK_PATH)/components/device)
INC_PATHS += -I$(abspath $(NRF_SDK_PATH)/components/drivers_nrf/common)
INC_PATHS += -I$(abspath $(NRF_SDK_PATH)/components/drivers_nrf/hal)
INC_PATHS += -I$(abspath $(NRF_SDK_PATH)/components/drivers_nrf/clock)
INC_PATHS += -I$(abspath $(NRF_SDK_PATH)/components/drivers_nrf/nrf_soc_nosd)
INC_PATHS += -I$(abspath $(NRF_SDK_PATH)/components/drivers_nrf/uart)
INC_PATHS += -I$(abspath $(NRF_SDK_PATH)/components/drivers_nrf/rtc)
INC_PATHS += -I$(abspath $(NRF_SDK_PATH)/components/drivers_nrf/ppi)
INC_PATHS += -I$(abspath $(NRF_SDK_PATH)/components/drivers_nrf/gpiote)
INC_PATHS += -I$(abspath $(NRF_SDK_PATH)/components/drivers_nrf/timer)
INC_PATHS += -I$(abspath $(NRF_SDK_PATH)/components/libraries/fifo)
INC_PATHS += -I$(abspath $(NRF_SDK_PATH)/components/libraries/uart)
INC_PATHS += -I$(abspath $(NRF_SDK_PATH)/components/libraries/util)
INC_PATHS += -I$(abspath $(NRF_SDK_PATH)/components/toolchain)
INC_PATHS += -I$(abspath $(NRF_SDK_PATH)/components/toolchain/CMSIS/Include)
INC_PATHS += -I$(abspath vendor/nrf51-micro-esb/common)

OBJECT_DIRECTORY = _build
LISTING_DIRECTORY = $(OBJECT_DIRECTORY)
OUTPUT_BINARY_DIRECTORY = $(OBJECT_DIRECTORY)

# Sorting removes duplicates
BUILD_DIRECTORIES := $(sort $(OBJECT_DIRECTORY) $(OUTPUT_BINARY_DIRECTORY) $(LISTING_DIRECTORY) )


# C compiler flags
CFLAGS := -DNRF51

CFLAGS += -mcpu=cortex-m0
CFLAGS += -mthumb -mabi=aapcs
CFLAGS += --std=c99
CFLAGS += -MD
CFLAGS += -Wall -Werror -O3 -g3
CFLAGS += -mfloat-abi=soft
# keep every function in separate section. This will allow linker to dump unused functions
CFLAGS += -ffunction-sections -fdata-sections -fno-strict-aliasing
CFLAGS += -fno-builtin --short-enums


# Assembler flags
ASMFLAGS += -x assembler-with-cpp
ASMFLAGS += -DNRF51


# Linker flags
LINKER_SCRIPT = linker.ld
LDFLAGS := -mcpu=cortex-m0
LDFLAGS += -mthumb -mabi=aapcs
LDFLAGS += -L$(TEMPLATE_PATH)
LDFLAGS += -T$(LINKER_SCRIPT)
LDFLAGS += -Xlinker -Map=$(LISTING_DIRECTORY)/$(PROJECT_NAME).map
LDFLAGS += -Wl,--gc-sections
# Use newlib in nano version
LDFLAGS += --specs=nano.specs -lc -lnosys



C_SOURCE_FILE_NAMES = $(notdir $(C_SOURCE_FILES))
C_PATHS = $(sort $(dir $(C_SOURCE_FILES) ) )
C_OBJECTS = $(addprefix $(OBJECT_DIRECTORY)/, $(C_SOURCE_FILE_NAMES:.c=.o) )

ASM_SOURCE_FILE_NAMES = $(notdir $(ASM_SOURCE_FILES))
ASM_PATHS = $(sort $(dir $(ASM_SOURCE_FILES) ))
ASM_OBJECTS = $(addprefix $(OBJECT_DIRECTORY)/, $(ASM_SOURCE_FILE_NAMES:.s=.o) )

vpath %.c $(C_PATHS)
vpath %.s $(ASM_PATHS)

OBJECTS = $(C_OBJECTS) $(ASM_OBJECTS)
$(OBJECTS): Makefile

###############################################################################
nrf51_sdk :
	$(ECHO)if [ ! "`ls -A $(NRF_SDK_PATH)/components/device/nrf51.h`" ] ; then \
		printf "\n"; \
		printf "######## ERROR ########\n"; \
		printf "The nRF51 SDK version nRF5_SDK_11.0.0_89a8197 is not accessible.\n"; \
		printf "\n"; \
		printf "Please download it from Nordic Semiconductors and create a:\n"; \
		printf "symlink named $(NRF_SDK_PATH) to it.\n"; \
		printf "\n\n"; \
		exit 1;\
		fi

###############################################################################

# Building all targets
all: $(OUTPUT_BINARY_DIRECTORY)/$(PROJECT_NAME).hex echosize

# Create build directories
$(BUILD_DIRECTORIES): nrf51_sdk
	@echo [MKDIR] $@
	$(ECHO)$(MK) $@

# Create objects from C SRC files
$(OBJECT_DIRECTORY)/%.o: %.c
	@echo [CC] $(notdir $<)
	$(ECHO)$(CC) $(CFLAGS) $(INC_PATHS) -c -o $@ $<

# Assemble files
$(OBJECT_DIRECTORY)/%.o: %.s
	@echo [AS] $(notdir $<)
	$(ECHO)$(CC) $(ASMFLAGS) $(INC_PATHS) -c -o $@ $<

# Link
$(OUTPUT_BINARY_DIRECTORY)/$(PROJECT_NAME).elf: $(BUILD_DIRECTORIES) $(OBJECTS)
	@echo [LD] $(PROJECT_NAME).elf
	$(ECHO)$(CC) $(LDFLAGS) $(OBJECTS) $(LIBS) -o $(OUTPUT_BINARY_DIRECTORY)/$(PROJECT_NAME).elf

# Create binary .bin file from the .elf file
$(OUTPUT_BINARY_DIRECTORY)/$(PROJECT_NAME).bin: $(OUTPUT_BINARY_DIRECTORY)/$(PROJECT_NAME).elf
	@echo [OBJCOPY] $(PROJECT_NAME).bin
	$(ECHO)$(OBJCOPY) -O binary $(OUTPUT_BINARY_DIRECTORY)/$(PROJECT_NAME).elf $(OUTPUT_BINARY_DIRECTORY)/$(PROJECT_NAME).bin

# Create binary .hex file from the .elf file
$(OUTPUT_BINARY_DIRECTORY)/$(PROJECT_NAME).hex: $(OUTPUT_BINARY_DIRECTORY)/$(PROJECT_NAME).elf
	@echo [OBJCOPY] $(PROJECT_NAME).hex
	$(ECHO)$(OBJCOPY) -O ihex $(OUTPUT_BINARY_DIRECTORY)/$(PROJECT_NAME).elf $(OUTPUT_BINARY_DIRECTORY)/$(PROJECT_NAME).hex

echosize:
	-@echo ''
	$(ECHO)$(SIZE) $(OUTPUT_BINARY_DIRECTORY)/$(PROJECT_NAME).elf
	-@echo ''

clean:
	$(ECHO)$(RM) $(BUILD_DIRECTORIES)

flash: program

program: $(OUTPUT_BINARY_DIRECTORY)/$(PROJECT_NAME).elf
	@echo [PROGRAM] $<
	$(ECHO)$(GDB) -nx -quiet -batch -command gdb.flash.script

debug: $(OUTPUT_BINARY_DIRECTORY)/$(PROJECT_NAME).elf
	$(ECHO) $(GDB) -nx -quiet -command gdb.debug.script

terminal:
	miniterm /dev/ttyUSB0 115200 --eol LF


.PHONY: nrf51_sdk clean program flash debug reset terminal echosize list all


###############################################################################
# Include (optional) dependency files. Needs -MD in CFLAGS
-include $(OBJECTS:.o=.d)
