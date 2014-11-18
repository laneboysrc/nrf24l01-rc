SOURCE_HEX := xr3100.hex
SOURCE_ASM := $(patsubst %.hex, %.asm, $(SOURCE_HEX))
SOURCE_DIR := ../../disassembly/

DASM := d52
DASM_OPTS :=

ASM := mcu8051ide
ASM_OPTS := --quiet --comp-quiet --nocolor --no-bin --no-sim
ASM_OPTS += --code-size 4K --xram-size 1K --assemble

MV := mv

CPPM_PATCHES := cppm.patch accurate_servo_pulses.patch


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
%.asm: $(SOURCE_DIR)%.d52 $(PATCHES)
	$(ECHO) [MV] $< -\> $@
	$(QUIET) $(MV) $< $@

$(SOURCE_DIR)%.d52: $(SOURCE_DIR)%.bin $(SOURCE_DIR)%.ctl
	$(ECHO) [DASM] $<
	$(QUIET) $(DASM) $(DASM_OPTS) $< >/dev/null

%.hex: %.asm
	$(ECHO) [ASM] $<
	$(QUIET) $(ASM) $(ASM_OPTS) $<


###############################################################################
all: cppm.hex

clean:
	$(ECHO) [RM] build files
	$(QUIET) $(RM) -f *~
	$(QUIET) $(RM) -f *.lst
	$(QUIET) $(RM) -f *.hex
	$(QUIET) $(RM) -f *.asm


###############################################################################
cppm.asm: $(SOURCE_ASM)
	$(ECHO) [CP] $< -\> $@
	$(QUIET) cp $< $@
	$(ECHO) [PATCH] $(CPPM_PATCHES)
	$(QUIET) for p in $(CPPM_PATCHES) ; do patch -s -i $$p ; done

cppm.hex: $(CPPM_PATCHES)


###############################################################################
.phony: all clean
