#include <types.h>
#include <sem.h>
#include <sfatfs.h>

void
lock_sfatfs_fs(struct sfatfs_fs *sfatfs) {
    down(&(sfatfs->fs_sem));
}

void
lock_sfatfs_io(struct sfatfs_fs *sfatfs) {
    down(&(sfatfs->io_sem));
}

void
lock_sfatfs_mutex(struct sfatfs_fs *sfatfs) {
    down(&(sfatfs->mutex_sem));
}

void
unlock_sfatfs_fs(struct sfatfs_fs *sfatfs) {
    up(&(sfatfs->fs_sem));
}

void
unlock_sfatfs_io(struct sfatfs_fs *sfatfs) {
    up(&(sfatfs->io_sem));
}

void
unlock_sfatfs_mutex(struct sfatfs_fs *sfatfs) {
    up(&(sfatfs->mutex_sem));
}

