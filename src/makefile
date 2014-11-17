SOURCE_HEX := xr3100.hex
SOURCE_ASM := $(patsubst %.hex, %.asm, $(SOURCE_HEX))
SOURCE_DIR := ../disassembly/

DASM := d52
DASM_OPTS :=

ASM := mcu8051ide
ASM_OPTS := --quiet --nocolor --no-bin --no-sim --assemble

MV := mv

CPPM_PATCHES := cppm.patch

%.asm: $(SOURCE_DIR)%.d52 $(PATCHES)
	$(MV) $< $@

$(SOURCE_DIR)%.d52: $(SOURCE_DIR)%.bin $(SOURCE_DIR)%.ctl
	$(DASM) $(DASM_OPTS) $<

%.hex: %.asm
	$(ASM) $(ASM_OPTS) $<



all: cppm.hex

cppm.asm: $(SOURCE_ASM)
	cp $< $@
	for p in $(CPPM_PATCHES) ; do patch < $$p ; done

cppm.hex: $(CPPM_PATCHES)

clean:
	$(RM) -f *~
	$(RM) -f *.lst
	$(RM) -f *.hex
	$(RM) -f *.asm

.phony: all bin clean


