#ifndef __DRIVERS_RAMDISK_H
#define __DRIVERS_RAMDISK_H

#include <ide.h>

void ramdisk_init(int devno, struct ide_device *dev);

#endif
