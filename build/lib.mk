
CYPRESS_SRCDIR  := ../config
CYPRESS_C_SRC   := $(notdir $(wildcard $(CYPRESS_SRCDIR)/*.c))
CYPRESS_A_SRC   := $(notdir $(CYPRESS_SRCDIR)/CyBootAsmGnu.s)
CYPRESS_OBJ     := $(CYPRESS_C_SRC:.c=.o) $(CYPRESS_A_SRC:.s=.o)

INCLUDES := -I $(CYPRESS_SRCDIR) -I ../fix_case

$(TARGET).a: $(CYPRESS_OBJ)
	$(AR) -rs $@ $^

%.o:$(CYPRESS_SRCDIR)/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

%.o:$(CYPRESS_SRCDIR)/%.s
	$(AS) $(ASMFLAGS) $(INCLUDES) $< -o $@ 
