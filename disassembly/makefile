OBJECTS := xr3100.d52 hk310.d52 hkr3000.d52

DASM := d52
DASM_OPTS := -d

ASM := mcu8051ide
ASM_OPTS := --quiet --nocolor --no-bin --no-sim --assemble

%.d52: %.bin %.ctl
	$(DASM) $(DASM_OPTS) $<

%.hex: %.d52
	$(ASM) $(ASM_OPTS) $<

all: $(OBJECTS)

clean:
	$(RM) -f *.lst*
	$(RM) -f *.hex
	$(RM) -f *.d52

.phony: all clean


