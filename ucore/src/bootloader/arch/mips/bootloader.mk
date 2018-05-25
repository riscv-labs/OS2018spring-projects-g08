BOOTSECT   := $(T_OBJ)/loader.bin

all: $(BOOTSECT)

$(BOOTSECT): arch/${ARCH}/bootasm.S
	${V}mkdir $(T_OBJ)
	$(CC) --fno-omit-frame-pointer -fno-optimize-sibling-calls -mno-sched-prolog -fno-builtin -nostdlib  -nostdinc -g  -EL -G0 -fno-delayed-branch -Wa,-O0 -c -o $(T_OBJ)/loader.o $^
	$(LD) -fno-omit-frame-pointer -fno-optimize-sibling-calls -mno-sched-prolog -EL -n -G0 -Ttext 0xbfc00000 -o $(T_OBJ)/loader $(T_OBJ)/loader.o
	$(OBJCOPY) -O binary  -S $(T_OBJ)/loader $@


.PHONY: all