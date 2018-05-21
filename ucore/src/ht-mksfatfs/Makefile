.PHONY: all clean

BE_ARCH	:= or32
ifneq (,$(findstring $(ARCH),$(BE_ARCH)))
OBJS := mksfatfs_be.c
else
OBJS := mksfatfs.c
endif
CFLAGS := -Wall -O2 -D_FILE_OFFSET_BITS=64

all: mksfatfs

mksfatfs: $(OBJS)
	$(CC) $(CFLAGS) -o $(OBJPATH_ROOT)/$@ $+


.PHONY: all