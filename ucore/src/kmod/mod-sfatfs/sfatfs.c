#include <sfatfs.h>
#include <error.h>
#include <assert.h>
#include <vfs.h>
#include <kio.h>

/*
void
sfatfs_init(void) {
    int ret;
    if ((ret = sfatfs_mount("disk1")) != 0) {
        panic("failed: sfatfs: sfatfs_mount: %e.\n", ret);
    }
}
*/

int init_mod() {
	int ret;
    kprintf("[ MM ] init mod-sfatfs\n");
    if ((ret = register_filesystem("sfatfs", sfatfs_mount)) != 0) {
    	kprintf("failed: sfatfs: register_filesystem: %e.\n", ret);
    } else
        kprintf("[ MM ] init mod-sfatfs done\n");
    return ret;
}

void cleanup_mod() {
	int ret;
    kprintf("[ MM ] cleanup mod-sfatfs\n");
    if ((ret = unregister_filesystem("sfatfs")) != 0) {
    	panic("failed: sfatfs: unregister_filesystem: %e.\n", ret);
    }
    kprintf("[ MM ] cleanup mod-sfatfs done\n");
}

