TESTBENCHES := step_dir_tb

WAVE_FORMAT := vcd

VL_COMPILE := iverilog
VL_RUN     := IVERILOG_DUMPER=$(WAVE_FORMAT) vvp

C_FLAGS	:= -I .. -I ../cypress

empty	:=
space	:= $(empty) $(empty)
newline := '\n'

all: step_dir_tb.vcd

list:
	@echo -e $(subst $(space),$(newline),$(TESTBENCHES:=.$(WAVE_FORMAT)))

deps_extract = $(shell tail $(1) | grep '\.v' | sed 's/.*\"\(.*\)\".*/\1 /')

-include $(addprefix deps/, $(TESTBENCHES:=.$(WAVE_FORMAT))) 

define build_tb =
    $(1).vvp: $$(addprefix ../testbench/, $(1).v)
	$$(VL_COMPILE) $$(C_FLAGS) -s $(1) -o $$@ \
		$$(addprefix -DWAVE_FILE=, $(1)).$$(WAVE_FORMAT) $$<
endef

define run_tb =
    $(1).$$(WAVE_FORMAT): $(1).vvp
	$$(eval deps = $$(call deps_extract, $$<))
	@mkdir -p deps
	@echo $$<: $$(deps) > deps/$$@
	$$(VL_RUN) $$<
endef

$(foreach testbench, $(TESTBENCHES), $(eval $(call build_tb, $(testbench))))
$(foreach testbench, $(TESTBENCHES), $(eval $(call run_tb,   $(testbench))))

clean:
	rm -rf deps
	rm -f *.vvp
	rm -f *.vcd
