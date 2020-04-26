TARGET	    := bootloader
TOP_SRCDIR  := ..

CYPRESS_DIR := $(TOP_SRCDIR)/$(TARGET).cydsn
  
CONFIG_DEPS := $(CYPRESS_DIR)/$(TARGET).cydwr \
	       $(CYPRESS_DIR)/$(TARGET).cyprj \
	       $(CYPRESS_DIR)/TopDesign/TopDesign.cysch

PROJECT_FILE  := $(CYPRESS_DIR)/$(TARGET).cyprj

SRC := main.c
OBJ := $(SRC:.c=.o)

CC := arm-psoc-eabi-gcc
AS := arm-psoc-eabi-as
AR := arm-psoc-eabi-ar

WINEPREFIX := ~/.local/share/wineprefixes/psoc_font
PC_BIN     := drive_c/Program\ Files/Cypress/PSoC\ Creator/4.2/PSoC\ Creator/bin
CYDSFIT	   := WINEPREFIX=$(WINEPREFIX) wine $(WINEPREFIX)/$(PC_BIN)/cydsfit

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
LDFLAGS += -Wl,-Map,$(TARGET).map -specs=nano.specs
LDFLAGS += -Wl,--gc-sections -g -ffunction-sections -Og -ffat-lto-objects

export

$(TARGET).code $(TARGET).nvl: $(TARGET).hex
	arm-psoc-eabi-objcopy $(TARGET).elf -O binary $@ \
	    -R .cyconfigecc --dump-section .cycustnvl=$(TARGET).nvl

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

config/cyfitter_cfg.c: $(CONFIG_DEPS)
	mkdir -p config
	cd config ; $(CYDSFIT) -p $(abspath $(PROJECT_FILE)) \
	    $(DSFIT_ARGS) -- $(WARP_ARGS) 2>/dev/null

%.o: $(TOP_SRCDIR)/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

cleanall: clean
	rm -f $(TARGET).map
	rm -f $(TARGET).elf
	rm -f $(TARGET).hex

clean:
	rm -rf config
	rm -rf lib
	rm -f *.o

.PHONY: clean
