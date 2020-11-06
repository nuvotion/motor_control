TESTBENCHES := vel_tb.vhd curpid_tb.vhd asrc_tb.vhd

VHDL_STD := 93
IEEE_LIB := standard

IMPORT_FLAGS 	:= --std=$(VHDL_STD) --ieee=$(IEEE_LIB)
ANALYSIS_FLAGS	:= --std=$(VHDL_STD) --ieee=$(IEEE_LIB)
#ANALYSIS_FLAGS	+= -fexplicit --warn-no-binding --warn-no-vital-generic
#ANALYSIS_FLAGS  += -frelaxed-rules

GHDL := ghdl

LOCAL_OBJS += \
	work-obj$(VHDL_STD).cf

all:
	@echo -e $(subst $(space),$(newline),$(TESTBENCHES:.vhd=))

ghdl_extract = $(shell grep $(1) $(2) | sed 's/.*$(1)://' | tr -d '\n')

-include $(addprefix deps/,$(TESTBENCHES:.vhd=.d))

deps/%.d: ../testbench/%.vhd
	@mkdir -p deps
	$(eval testbench = $(notdir $(basename $@)))
	$(eval flags = $(call ghdl_extract, ghdl_flags, $<))
	$(eval deps = $(call ghdl_extract, ghdl_deps, $<))
	@echo $(testbench): $(addprefix ../, $(deps)) $< > $@
	@echo $(testbench)_deps  += $(addprefix ../, $(deps)) >> $@
	@echo $(testbench)_flags += $(flags) >> $@

%_tb: $(LOCAL_OBJS)
	$(eval dep_list = $@_deps)
	$(GHDL) -i $(IMPORT_FLAGS) ../testbench/$@.vhd $($@_deps)
	$(GHDL) -m $(ANALYSIS_FLAGS) $@
	./$@ $($@_flags)

work-obj$(VHDL_STD).cf:

clean:
	$(GHDL) --remove
cleanall:
	$(GHDL) --remove
	rm -rf deps
	rm -f *.ghw
	rm -f *.cf
	rm -f *.o

.PHONY: clean cleanall all

newline := \\n
empty :=
space := $(empty) $(empty)

#ghdl -e -Wc,-fdump-tree-gimple
