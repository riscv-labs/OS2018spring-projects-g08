#include <assert.h>
#include <types.h>
#include <fs.h>
#include <ramdisk.h>
#include <stdio.h>
#include <kio.h>
#include <string.h>
#include <memlayout.h>


#define MIN(x, y) (((x) < (y)) ? (x) : (y))


bool check_initrd() {
    if (initrd_begin == initrd_end) {
        kprintf("Warning: No Initrd!\n");
        return 0;
    }
    kprintf("Initrd: 0x%08x - 0x%08x, size: 0x%08x\n", initrd_begin,
            initrd_end - 1, initrd_end - initrd_begin);
    return 1;
}

static int ramdisk_read(struct ide_device *dev, size_t secno, void *dst,
                        size_t nsecs) {
    nsecs = MIN(nsecs, dev->size - secno);
    if (nsecs < 0) return -1;
    memcpy(dst, (void *)(dev->iobase + secno * SECTSIZE), nsecs * SECTSIZE);
    return 0;
}

static int ramdisk_write(struct ide_device *dev, size_t secno, const void *src,
                         size_t nsecs) {
    nsecs = MIN(nsecs, dev->size - secno);
    if (nsecs < 0) return -1;
    memcpy((void *)(dev->iobase + secno * SECTSIZE), src, nsecs * SECTSIZE);
    return 0;
}

void ramdisk_init(struct ide_device *dev) {
    memset(dev, 0, sizeof(struct ide_device));
    assert(INITRD_SIZE() % SECTSIZE == 0);
    // char *_initrd_begin;
    // char *_initrd_end;
    // if (devno == SWAP_DEV_NO) {
    //     _initrd_begin = _binary_swap_img_start;
    //     _initrd_end = _binary_swap_img_end;
    // } else if (devno == DISK0_DEV_NO) {
    //     _initrd_begin = _binary_sfs_img_start;
    //     _initrd_end = _binary_sfs_img_end;
    // } else {
    //     panic("Device Not Found");
    // }
    if (CHECK_INITRD_EXIST()) {
		dev->valid = 1;
		dev->sets = ~0;
        dev->size = (unsigned int)INITRD_SIZE() / SECTSIZE;
        dev->iobase = (void *) DISK_FS_VBASE;
        strcpy(dev->model, "KERN_INITRD");
        dev->read_secs = ramdisk_read;
        dev->write_secs = ramdisk_write;
	}
}
