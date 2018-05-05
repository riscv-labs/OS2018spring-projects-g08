#ifndef __DRIVERS_RAMDISK_H
#define __DRIVERS_RAMDISK_H

#include <ide.h>

/* defined in ldscript */
extern char initrd_begin[], initrd_end[];
extern char swaprd_begin[], swaprd_end[];

bool check_initrd();
bool check_swaprd();

#define CHECK_INITRD_EXIST() (initrd_end != initrd_begin)
#define INITRD_SIZE() (initrd_end-initrd_begin)

#define CHECK_SWAPRD_EXIST() (swaprd_end != swaprd_begin)
#define SWAPRD_SIZE() (swaprd_end-swaprd_begin)

void ramdisk_init(int devno, struct ide_device *dev);

#endif
