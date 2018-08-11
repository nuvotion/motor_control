TARGET	    := bootloader
TOP_SRCDIR  := ..

CYPRESS_DIR := $(TOP_SRCDIR)/bootloader.cydsn
  
CONFIG_DEPS := $(CYPRESS_DIR)/$(TARGET).cydwr \
	       $(CYPRESS_DIR)/$(TARGET).cyprj \
	       $(CYPRESS_DIR)/TopDesign/TopDesign.cysch

PROJECT_FILE  := $(CYPRESS_DIR)/$(TARGET).cyprj
WIN_PROJ_FILE := $(subst \,\\,$(shell cygpath -w $(PROJECT_FILE)))

SRC := main.c
OBJ := $(SRC:.c=.o)

CC := arm-none-eabi-gcc
AS := arm-none-eabi-as

INCLUDES := -I config

CFLAGS := -mcpu=cortex-m3 -mthumb -g -Wall
CFLAGS += -ffunction-sections -ffat-lto-objects -Og
CFLAGS += -D DEBUG -D CY_CORE_ID=0 

ASMFLAGS := -mcpu=cortex-m3 -mthumb -g -W

DSFIT_ARGS := -.fdswarpdepfile=warp_dependencies.txt
DSFIT_ARGS += -.fdselabdepfile=elab_dependencies.txt
DSFIT_ARGS += -.fdsbldfile=generated_files.txt
DSFIT_ARGS += -.fdsreffile=referenced_files.txt

# Debug verbosity 2, quiet mode, optimise for speed, maximum optimisation,
# virtual substitution level 3, big endian FFT?
WARP_ARGS := -yv2 -q10 -ygs -o2 -v3 -.fftcfgtype=LE

LDFLAGS := -mcpu=cortex-m3 -mthumb -L config -T config/cm3gcc.ld
LDFLAGS += -Wl,-Map,bootloader.map -specs=nano.specs
LDFLAGS += -Wl,--gc-sections -g -ffunction-sections -Og -ffat-lto-objects

export

$(TARGET).hex: $(TARGET).elf
	cyelftool -P $< \
	    --flash_row_size 256 \
	    --flash_size 262144 \
	    --flash_offset 0x00000000 \
	    --size_var_name Bootloader_SizeBytes \
	    --checksum_var_name Bootloader_Checksum
	cyelftool -S $<

$(TARGET).elf: lib/$(TARGET).a $(OBJ)
	$(CC) $(LDFLAGS) $^ -o $@

lib/$(TARGET).a: config/cyfitter_cfg.c
	mkdir -p lib
	$(MAKE) -C lib -f $(TOP_SRCDIR)/lib.mk $(TARGET).a

config/cyfitter_cfg.c: $(PROJECT_FILE) $(CONFIG_DEPS)
	mkdir -p config
	cd config ; cydsfit -p $(WIN_PROJ_FILE) $(DSFIT_ARGS) -- $(WARP_ARGS)

%.o: $(TOP_SRCDIR)/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -rf config
	rm -rf lib
	rm -f *.o
	rm -f $(TARGET).map
	rm -f $(TARGET).elf
	rm -f $(TARGET).hex

.PHONY: clean
