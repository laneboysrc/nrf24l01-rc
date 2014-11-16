OBJECTS := xr3100.d52 hk310.d52 hkr3000.d52

DASM := d52
DASM_OPTS := -d

ASM := mcu8051ide
ASM_OPTS := --quiet --nocolor --no-bin --no-sim --assemble

REASSEMBLY_SUFFIX := -reassembly

REASSEMBLED_OBJECTS := $(patsubst %.d52, %$(REASSEMBLY_SUFFIX).bin, $(OBJECTS))




%.d52: %.bin %.ctl
	$(DASM) $(DASM_OPTS) $<

%.hex: %.d52
	$(ASM) $(ASM_OPTS) $<

%$(REASSEMBLY_SUFFIX).bin: %.hex
	objcopy --gap-fill=0 -I ihex -O binary $< $@

all: $(OBJECTS)

bin: $(REASSEMBLED_OBJECTS)

clean:
	$(RM) -f *.lst*
	$(RM) -f *.hex
	$(RM) -f *$(REASSEMBLY_SUFFIX).bin
	$(RM) -f *.d52

.phony: all bin clean


