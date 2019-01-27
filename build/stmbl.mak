stmbl_obj_name = $(addprefix stmbl/, $(notdir $(1:.c=.o)))
define stmbl_lib_dep
    $(eval $$(call stmbl_obj_name, $1): $1 $(STMBL_TBL)
	@mkdir -p stmbl
	$(CC) $(CFLAGS) $(INCLUDES) -c $$< -o $$@)
endef
$(foreach obj, $(STMBL_SRC), $(call stmbl_lib_dep, $(obj)))

stmbl/commands.o: stmbl/inc/commandslist.h

stmbl/stmbl.a: $(call stmbl_obj_name, $(STMBL_SRC))
	$(AR) -rs $@ $^

stmbl/inc/commandslist.h: $(STMBL_SRC)
	@mkdir -p stmbl/inc
	$(PYTHON) $(STMBL_DIR)/tools/create_cmd.py $^ > $@

stmbl/inc/hal_tbl.h stmbl/src/hal_tbl.c: $(STMBL_COMPS)
	@mkdir -p stmbl/src stmbl/inc
	$(PYTHON) $(STMBL_DIR)/tools/create_hal_tbl.py stmbl $(STMBL_COMPS)

