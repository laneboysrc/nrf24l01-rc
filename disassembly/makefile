

OBJECTS := xr3100.d52 hk310.d52

DASM := d52
DASM_OPTS := -d

%.d52: %.hex %.ctl
	$(DASM) $(DASM_OPTS) $<

all: $(OBJECTS)

.phony: all
