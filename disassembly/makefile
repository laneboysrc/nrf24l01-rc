OBJECTS := xr3100.d52 hk310.d52

DASM := d52
DASM_OPTS := -d

ASM := mcu8051ide
ASM_OPTS := --quiet --nocolor --no-hex --no-sim --assemble

%.d52: %.hex %.ctl
	$(DASM) $(DASM_OPTS) $<

%.bin: %.d52
	$(ASM) $(ASM_OPTS) $<

all: $(OBJECTS)

clean:
	$(RM) -f *.lst*
	$(RM) -f *.bin

.phony: all clean


