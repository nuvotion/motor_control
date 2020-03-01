TARGET	    := application
TOP_SRCDIR  := ..
STMBL_DIR   := $(TOP_SRCDIR)/stmbl_fixed

CYPRESS_DIR := $(TOP_SRCDIR)/$(TARGET).cydsn
  
CONFIG_DEPS := $(CYPRESS_DIR)/$(TARGET).cydwr \
	       $(CYPRESS_DIR)/$(TARGET).cyprj \
	       $(CYPRESS_DIR)/TopDesign/TopDesign.cysch

PROJECT_FILE  := $(CYPRESS_DIR)/$(TARGET).cyprj

STMBL_TBL := stmbl/inc/hal_tbl.h stmbl/src/hal_tbl.c
#STMBL_COMPS := $(STMBL_DIR)/shared/comps/uvw.c
#STMBL_COMPS += $(STMBL_DIR)/shared/comps/fb_switch.c
#STMBL_COMPS += $(STMBL_DIR)/shared/comps/vel.c
#STMBL_COMPS += $(STMBL_DIR)/shared/comps/pid.c
#STMBL_COMPS += $(STMBL_DIR)/shared/comps/pmsm_ttc.c
#STMBL_COMPS += $(STMBL_DIR)/shared/comps/pmsm_limits.c
#STMBL_COMPS += $(STMBL_DIR)/shared/comps/dq.c
#STMBL_COMPS += $(STMBL_DIR)/shared/comps/curpid.c
#STMBL_COMPS += $(STMBL_DIR)/shared/comps/idq.c
#STMBL_COMPS += $(STMBL_DIR)/shared/comps/svm.c
STMBL_COMPS += $(TOP_SRCDIR)/adc.c
#STMBL_COMPS += $(TOP_SRCDIR)/pwm.c
#STMBL_COMPS += $(TOP_SRCDIR)/encoder.c
#STMBL_COMPS += $(TOP_SRCDIR)/step_dir.c
STMBL_COMPS += $(TOP_SRCDIR)/dbg_comp.c
STMBL_SRC := $(STMBL_COMPS)
STMBL_SRC += $(STMBL_DIR)/shared/angle.c
STMBL_SRC += stmbl/src/hal_tbl.c
STMBL_SRC += $(STMBL_DIR)/shared/hal.c

SETUPS := $(TOP_SRCDIR)/setup_normal.c
SETUPS += $(TOP_SRCDIR)/setup_com_test.c
SETUPS += $(TOP_SRCDIR)/setup_curpid_test.c
SETUPS += $(TOP_SRCDIR)/setup_cur_gain.c

SRC := main.c print.c
OBJ := $(SRC:.c=.o)

CC := arm-psoc-eabi-gcc
AS := arm-psoc-eabi-as
AR := arm-psoc-eabi-ar
ifneq ($(shell uname),Linux)
    CYELFTOOL := cyelftool
    WIN_PROJ_FILE := $(subst \,\\,$(shell cygpath -w $(PROJECT_FILE)))
else
    TOOLCHAIN := $(abspath $(TOP_SRCDIR)/../toolchain)
    CYELFTOOL := wine $(TOOLCHAIN)/cyelftool
endif

STMBL_INC := -I stmbl/inc -I $(STMBL_DIR)/shared
INCLUDES  := -I config $(STMBL_INC) -I fix_case

CFLAGS := -mcpu=cortex-m3 -mthumb -Wall -std=gnu11 -g
CFLAGS += -ffunction-sections -ffat-lto-objects -O3 -flto
CFLAGS += -D DEBUG -D CY_CORE_ID=0 
#CFLAGS += -D COM_TEST
#CFLAGS += -D CURPID_TEST
CFLAGS += -D FIXED_POINT_TEST

ASMFLAGS := -mcpu=cortex-m3 -mthumb -W

DSFIT_ARGS := -.fdswarpdepfile=warp_dependencies.txt
DSFIT_ARGS += -.fdselabdepfile=elab_dependencies.txt
DSFIT_ARGS += -.fdsbldfile=generated_files.txt
DSFIT_ARGS += -.fdsreffile=referenced_files.txt

# Debug verbosity 2, quiet mode, optimise for speed, maximum optimisation,
# virtual substitution level 3, big endian FFT?
WARP_ARGS := -yv2 -q10 -ygs -o2 -v3 -.fftcfgtype=LE

LDFLAGS := -mcpu=cortex-m3 -mthumb -L config -T config/cm3gcc.ld
LDFLAGS += -Wl,-Map,$(TARGET).map -specs=nano.specs -flto
LDFLAGS += -Wl,--gc-sections -ffunction-sections -ffat-lto-objects -O3 -g
LDFLAGS += -u _printf_float
#LDFLAGS += -u _scanf_float

export

$(TARGET).cyacd: $(TARGET).elf
	$(CYELFTOOL) -B $< \
	    --flash_row_size 256 \
	    --flash_size 262144 \
	    --flash_offset 0x00000000 \
	    --flash_array_size 65536 \
	    --ee_array 64 \
	    --ee_array_size 2048 \
	    --ee_row_size 16
	$(CYELFTOOL) -S $<

$(TARGET).elf: $(OBJ) stmbl/stmbl.a lib/$(TARGET).a
	$(CC) $(LDFLAGS) $(OBJ) stmbl/stmbl.a lib/$(TARGET).a -lm -o $@

lib/$(TARGET).a: config/cyfitter_cfg.c
	mkdir -p lib
	$(MAKE) -C lib -f $(TOP_SRCDIR)/lib.mk $(TARGET).a

config/cyfitter_cfg.c: $(CONFIG_DEPS)
	mkdir -p config
	cd config ; cydsfit -p $(WIN_PROJ_FILE) $(DSFIT_ARGS) -- $(WARP_ARGS)

%.o: $(TOP_SRCDIR)/%.c $(STMBL_TBL)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

main.o: $(TOP_SRCDIR)/main.c $(STMBL_TBL) $(SETUPS)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

include stmbl.mak

cleanall: clean
	rm -rf config

clean:
	rm -rf lib
	rm -rf stmbl
	rm -f *.o
	rm -f $(TARGET).map
	rm -f $(TARGET).elf
	rm -f $(TARGET).hex
	rm -f $(TARGET).cyacd

.PHONY: clean
