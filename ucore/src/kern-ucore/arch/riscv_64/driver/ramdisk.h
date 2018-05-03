#ifndef __DRIVERS_RAMDISK_H
#define __DRIVERS_RAMDISK_H

#include <ide.h>

/* defined in ldscript */
extern char initrd_begin[], initrd_end[];

bool check_initrd();

#define CHECK_INITRD_EXIST() (initrd_end != initrd_begin)
#define INITRD_SIZE() (initrd_end-initrd_begin)

void ramdisk_init(struct ide_device *dev);

#endif
