#include <types.h>
#include <string.h>
#include <dev.h>
#include <sfatfs.h>
#include <iobuf.h>
#include <bitmap.h>
#include <assert.h>

static int
sfatfs_rwblock_nolock(struct sfatfs_fs *sfatfs, void *buf, uint32_t blkno, bool write, bool check) {
    assert((1) && blkno < sfatfs->super.blocks);
    struct iobuf __iob, *iob = iobuf_init(&__iob, buf, SFATFS_BLKSIZE, blkno * SFATFS_BLKSIZE);
    return dop_io(sfatfs->dev, iob, write);
}

static int
sfatfs_rwblock(struct sfatfs_fs *sfatfs, void *buf, uint32_t blkno, uint32_t nblks, bool write) {
    int ret = 0;
    lock_sfatfs_io(sfatfs);
    {
        while (nblks != 0) {
            if ((ret = sfatfs_rwblock_nolock(sfatfs, buf, blkno, write, 1)) != 0) {
                break;
            }
            blkno ++, nblks --;
            buf += SFATFS_BLKSIZE;
        }
    }
    unlock_sfatfs_io(sfatfs);
    return ret;
}

int
sfatfs_rblock(struct sfatfs_fs *sfatfs, void *buf, uint32_t blkno, uint32_t nblks) {
    return sfatfs_rwblock(sfatfs, buf, blkno, nblks, 0);
}

int
sfatfs_wblock(struct sfatfs_fs *sfatfs, void *buf, uint32_t blkno, uint32_t nblks) {
    return sfatfs_rwblock(sfatfs, buf, blkno, nblks, 1);
}

int
sfatfs_rbuf(struct sfatfs_fs *sfatfs, void *buf, size_t len, uint32_t blkno, off_t offset) {
    assert(offset >= 0 && offset < SFATFS_BLKSIZE && offset + len <= SFATFS_BLKSIZE);
    int ret;
    lock_sfatfs_io(sfatfs);
    {
        if ((ret = sfatfs_rwblock_nolock(sfatfs, sfatfs->sfatfs_buffer, blkno, 0, 1)) == 0) {
            memcpy(buf, sfatfs->sfatfs_buffer + offset, len);
        }
    }
    unlock_sfatfs_io(sfatfs);
    return ret;
}

int
sfatfs_wbuf(struct sfatfs_fs *sfatfs, void *buf, size_t len, uint32_t blkno, off_t offset) {
    assert(offset >= 0 && offset < SFATFS_BLKSIZE && offset + len <= SFATFS_BLKSIZE);
    int ret;
    lock_sfatfs_io(sfatfs);
    {
        if ((ret = sfatfs_rwblock_nolock(sfatfs, sfatfs->sfatfs_buffer, blkno, 0, 1)) == 0) {
            memcpy(sfatfs->sfatfs_buffer + offset, buf, len);
            ret = sfatfs_rwblock_nolock(sfatfs, sfatfs->sfatfs_buffer, blkno, 1, 1);
        }
    }
    unlock_sfatfs_io(sfatfs);
    return ret;
}

int
sfatfs_sync_super(struct sfatfs_fs *sfatfs) {
    int ret = 0;
    lock_sfatfs_io(sfatfs);
    {
        memset(sfatfs->sfatfs_buffer, 0, SFATFS_BLKSIZE);
		/* Copy the super */
        memcpy(sfatfs->sfatfs_buffer, &(sfatfs->super), sizeof(sfatfs->super));
		/* Copy file entries */
		memcpy(sfatfs->sfatfs_buffer + SFATFS_ROOTDIR_OFFSET, sfatfs->dinos, sfatfs->super.ndirentry * sizeof(struct sfatfs_disk_inode));
        ret = sfatfs_rwblock_nolock(sfatfs, sfatfs->sfatfs_buffer, SFATFS_BLKN_SUPER, 1, 0);
    }
    unlock_sfatfs_io(sfatfs);
    return ret;
}

int
sfatfs_sync_freemap(struct sfatfs_fs *sfatfs) {
    return sfatfs_wblock(sfatfs, sfatfs->fat, SFATFS_BLKN_FAT, 1);
}

int
sfatfs_clear_block(struct sfatfs_fs *sfatfs, uint32_t blkno, uint32_t nblks) {
    int ret = 0;
    lock_sfatfs_io(sfatfs);
    {
        memset(sfatfs->sfatfs_buffer, 0, SFATFS_BLKSIZE);
        while (nblks != 0) {
            if ((ret = sfatfs_rwblock_nolock(sfatfs, sfatfs->sfatfs_buffer, blkno, 1, 1)) != 0) {
                break;
            }
            blkno ++, nblks --;
        }
    }
    unlock_sfatfs_io(sfatfs);
    return ret;
}

