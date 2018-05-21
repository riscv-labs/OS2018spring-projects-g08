#include <types.h>
#include <string.h>
#include <stdlib.h>
#include <slab.h>
#include <list.h>
#include <stat.h>
#include <vfs.h>
#include <dev.h>
#include <sfatfs.h>
#include <inode.h>
#include <iobuf.h>
#include <bitmap.h>
#include <error.h>
#include <assert.h>
//Op mapping
static const struct inode_ops sfatfs_node_dirops;
static const struct inode_ops sfatfs_node_fileops;
// Lock the inode
static void
lock_sin(struct sfatfs_inode *sin) {
    down(&(sin->sem));
}
//Unlock the inode
static void
unlock_sin(struct sfatfs_inode *sin) {
    up(&(sin->sem));
}
///Return the op mapping
static const struct inode_ops *
sfatfs_get_ops(uint8_t attr) {
	if (attr & SFATFS_TYPE_DIR_MASK)
        return &sfatfs_node_dirops;
	else if (attr & SFATFS_TYPE_FILE_MASK)
        return &sfatfs_node_fileops;
    panic("invalid file attr %d.\n", attr);
}
/*
 * Create a new inode with index 'ino'. (Allocate and init)
 * If allocate failed, return no_mem
 */
static int
sfatfs_create_inode(struct sfatfs_fs *sfatfs, struct sfatfs_disk_inode *din, uint32_t ino, struct inode **node_store) {
    struct inode *node;
    if ((node = alloc_inode(sfatfs_inode)) != NULL) {
        vop_init(node, sfatfs_get_ops(din->attr), info2fs(sfatfs, sfatfs));
        
        struct sfatfs_inode *sin = vop_info(node, sfatfs_inode);
        sin->din = din, sin->ino = ino, sin->dirty = 0, sin->reclaim_count = 1;
        sem_init(&(sin->sem), 1);
        *node_store = node;
        return 0;
    }
    return -E_NO_MEM;
}
/*
 * Load the inode with index 'ino'.
 * This version of the function is very simple cos we don't cache inode and all the disk_inodes is loaded into memory when the fs is constructed.
 * Otherwise, we should firstly do a cache lookup,if the inode is not found in cache, we then load the disk inode from hard drive and call sfatfs_ create_inode().See sfs_load_inode();
 */
int
sfatfs_load_inode(struct sfatfs_fs *sfatfs, struct inode **node_store, uint32_t ino) {
    struct inode *node;
    int ret;
    struct sfatfs_disk_inode *din = sfatfs->dinos + ino;

    if ((ret = sfatfs_create_inode(sfatfs, din, ino, &node)) != 0) {
		panic("create inode failed, ino=%d", ino);
    }
	*node_store = node;
    return ret;
}
/*
 * Find a free block by checking the FAT
 * One block is free iff it's FAT status equals FAT_FREE
 */
static int
sfatfs_bmap_findfree_nolock(struct sfatfs_fs *sfatfs, uint16_t *bno_store){
	int i;
	for (i = 0; i < SFATFS_BLK_NENTRY; i++)
		if (*(sfatfs->fat + i) == FAT_FREE) {
			*bno_store = i;
			return 0;
		}
	return -E_NO_MEM;
}
/*
 * Find the actual block number on disk for the inode's 'index'th block.
 * All the blocks used by one inode are linked togather and numbered from 0 to inode.blocks - 1.
 * The 'index' could be 0 - inode.blocks. 
 * If 'index' == inode.blocks, we should find a free block and append it to the inode's linked list.
 */
static int
sfatfs_bmap_load_nolock(struct sfatfs_fs *sfatfs, struct sfatfs_inode *sin, uint32_t index, uint32_t *ino_store) {
	uint16_t pclu = sin->din->first_cluster;
	int i;
	for (i = 0; i < index; i++) {
		if (sfatfs->fat[pclu] == FAT_EOF) {
			if (i == index - 1){ //append one more block
				int ret;
				uint16_t ino;
				if ((ret = sfatfs_bmap_findfree_nolock(sfatfs, &ino)) != 0){
					return ret;
				}
				//append the new block to the end of the linked list
				sfatfs->fat[pclu] = ino;
				sfatfs->fat[ino] = FAT_EOF;
				pclu = ino;
				break;
			} else return -E_INVAL;//index is too large
		}
		pclu = sfatfs->fat[pclu];//travel the FAT link
	}
	if (ino_store != NULL) *ino_store = pclu;
    return 0;
}
/*
 * Truncate the last one block in the linked list of the inode.
 * We also remain at least one block for an empty file.
 */
static int
sfatfs_bmap_truncate_nolock(struct sfatfs_fs *sfatfs, struct sfatfs_inode *sin) {
	uint16_t pclu = sin->din->first_cluster;
	assert(pclu >= 0 && pclu < SFATFS_BLK_NENTRY);
	if (sfatfs->fat[pclu] == FAT_EOF) {
		sin->din->size = 0;
		sin->dirty = 1;
		//sfatfs->fat[pclu] = FAT_FREE;// remain at least one block for the empty file
		return 0;
	}
	//find the last block & free it
	uint16_t nclu = sfatfs->fat[pclu];
	while (sfatfs->fat[nclu] != FAT_EOF) {
		pclu = nclu;
		nclu = sfatfs->fat[pclu];
	}
	sfatfs->fat[pclu] = FAT_EOF;
	sfatfs->fat[nclu] = FAT_FREE;
	sin->dirty = 1;
    return 0;
}
/*
 * Search the dir to find the inode's number for the given filename.
 * Some parameters here are unused but reserved, cos we must provide the same interface to VFS.
 */
static int
sfatfs_dirent_search_nolock(struct sfatfs_fs *sfatfs, struct sfatfs_inode *sin, const char *name, uint32_t *ino_store, int *slot, int *empty_slot) {
   	uint32_t len = strlen(name);
	assert(len <= SFATFS_MAX_FNAME_LEN);
	uint32_t count = sfatfs->super.ndirentry;
	struct sfatfs_disk_inode *ino;
	while (count --) {
		ino = sfatfs->dinos + count;
		//Memcmp, not strcmp, cause the filename within one disk inode is not ending with '\0'
		if (!memcmp(ino->name, name, len)){
			if (len == SFATFS_MAX_FNAME_LEN || ino->name[len] == 0) { //Found!
				*ino_store = count; 
				return 0;
			}
		}
	}
	return -E_NOENT;
}
/*
 * Lookup the filename and load the corresponding inode to 'node_store'.
 */
static int
sfatfs_lookup_once(struct sfatfs_fs *sfatfs, struct sfatfs_inode *sin, const char *name, struct inode **node_store, int *slot) {
    int ret;
    uint32_t ino;
    lock_sin(sin);
    {
        ret = sfatfs_dirent_search_nolock(sfatfs, sin, name, &ino, slot, NULL);
    }
    unlock_sin(sin);
    if (ret == 0) { //found the file & got the ino
        ret = sfatfs_load_inode(sfatfs, node_store, ino);
    }
    return ret;
}
//Implement VFS interface
//Only O_RDONLY is supported for dir open mode
static int
sfatfs_opendir(struct inode *node, uint32_t open_flags) {
    switch (open_flags & O_ACCMODE) {
    case O_RDONLY:
        break;
    case O_WRONLY:
    case O_RDWR:
    default:
        return -E_ISDIR;
    }
    if (open_flags & O_APPEND) {
        return -E_ISDIR;
    }
    return 0;
}

//Implement VFS interface, do nothing
static int
sfatfs_openfile(struct inode *node, uint32_t open_flags) {
    return 0;
}

/* 
 * Do sync when close.
 * Implement VFS interface
 */
static int
sfatfs_close(struct inode *node) {
    return vop_fsync(node);
}

//Do io to IO_BUF buf with no lock
static int
sfatfs_io_nolock(struct sfatfs_fs *sfatfs, struct sfatfs_inode *sin, void *buf, off_t offset, size_t *alenp, bool write) {
    struct sfatfs_disk_inode *din = sin->din;
    assert(!(din->attr & SFATFS_TYPE_DIR_MASK));
    //init & check
    off_t endpos = offset + *alenp, blkoff;
    *alenp = 0;
    if (offset < 0 || offset >= SFATFS_MAX_FILE_SIZE || offset > endpos) {
        return -E_INVAL;
    }
    if (offset == endpos) {
        return 0;
    }
    if (endpos > SFATFS_MAX_FILE_SIZE) {
        endpos = SFATFS_MAX_FILE_SIZE;
    }
    if (!write) {
        if (offset >= din->size) {
            return 0;
        }
        if (endpos > din->size) {
            endpos = din->size;
        }
    }
    //read or write
    int (*sfatfs_buf_op)(struct sfatfs_fs *sfatfs, void *buf, size_t len, uint32_t blkno, off_t offset);
    int (*sfatfs_block_op)(struct sfatfs_fs *sfatfs, void *buf, uint32_t blkno, uint32_t nblks);
    if (write) {
        sfatfs_buf_op = sfatfs_wbuf, sfatfs_block_op = sfatfs_wblock;
    }
    else {
        sfatfs_buf_op = sfatfs_rbuf, sfatfs_block_op = sfatfs_rblock;
    }
    //DO io block by block
    int ret = 0;
    size_t size, alen = 0;
    uint32_t ino;
    uint32_t blkno = offset / SFATFS_BLKSIZE;
    uint32_t nblks = endpos / SFATFS_BLKSIZE - blkno;
    //head block
    if ((blkoff = offset % SFATFS_BLKSIZE) != 0) {
        size = (nblks != 0) ? (SFATFS_BLKSIZE - blkoff) : (endpos - offset);
        if ((ret = sfatfs_bmap_load_nolock(sfatfs, sin, blkno, &ino)) != 0) {
            goto out;
        }
        if ((ret = sfatfs_buf_op(sfatfs, buf, size, ino, blkoff)) != 0) {
            goto out;
        }
        alen += size;
        if (nblks == 0) {
            goto out;
        }
        buf += size, blkno ++, nblks --;
    }
  //mid blocks
    size = SFATFS_BLKSIZE;
    while (nblks != 0) {
        if ((ret = sfatfs_bmap_load_nolock(sfatfs, sin, blkno, &ino)) != 0) {
            goto out;
        }
        if ((ret = sfatfs_block_op(sfatfs, buf, ino, 1)) != 0) {
            goto out;
        }
        alen += size, buf += size, blkno ++, nblks --;
    }
  //tail block
    if ((size = endpos % SFATFS_BLKSIZE) != 0) {
        if ((ret = sfatfs_bmap_load_nolock(sfatfs, sin, blkno, &ino)) != 0) {
            goto out;
        }
        if ((ret = sfatfs_buf_op(sfatfs, buf, size, ino, 0)) != 0) {
            goto out;
        }
        alen += size;
    }

out:
  //update meta data
    *alenp = alen;
    if (offset + alen > sin->din->size) {
        sin->din->size = offset + alen;
        sin->dirty = 1;
    }
    return ret;
}

//Lock and do IO, calls sfatfs_io_nolock()
static inline int
sfatfs_io(struct inode *node, struct iobuf *iob, bool write) {
    struct sfatfs_fs *sfatfs = fsop_info(vop_fs(node), sfatfs);
    struct sfatfs_inode *sin = vop_info(node, sfatfs_inode);
    int ret;
    lock_sin(sin);
    {
        size_t alen = iob->io_resid;
        ret = sfatfs_io_nolock(sfatfs, sin, iob->io_base, iob->io_offset, &alen, write);
        if (alen != 0) {
            iobuf_skip(iob, alen);
        }
    }
    unlock_sin(sin);
    return ret;
}
//Do read, vfs_op
static int
sfatfs_read(struct inode *node, struct iobuf *iob) {
    return sfatfs_io(node, iob, 0);
}
//Do write, vfs_op
static int
sfatfs_write(struct inode *node, struct iobuf *iob) {
    return sfatfs_io(node, iob, 1);
}

//Get the inode's stat
static int
sfatfs_fstat(struct inode *node, struct stat *stat) {
    int ret;
    memset(stat, 0, sizeof(struct stat));
    if ((ret = vop_gettype(node, &(stat->st_mode))) != 0) {
        return ret;
    }
    struct sfatfs_disk_inode *din = vop_info(node, sfatfs_inode)->din;
    stat->st_size = din->size;
	stat->st_nlinks = 1;// FAT fs has no link
	stat->st_blocks = din->size / SFATFS_BLKSIZE;
	stat->st_blocks += din->size % SFATFS_BLKSIZE > 0 ? 1 : 0;
    return 0;
}
//sync one inode
static int
sfatfs_fsync(struct inode *node) {
    struct sfatfs_fs *sfatfs = fsop_info(vop_fs(node), sfatfs);
    struct sfatfs_inode *sin = vop_info(node, sfatfs_inode);
    int ret = 0;
    if (sin->dirty) {
        lock_sin(sin);
        {
            if (sin->dirty) {
                sin->dirty = 0;
                if ((ret = sfatfs_wbuf(sfatfs, sin->din, sizeof(struct sfatfs_disk_inode), SFATFS_BLKN_SUPER, SFATFS_ROOTDIR_OFFSET + sin->ino * sizeof(struct sfatfs_disk_inode))) != 0) {
                    sin->dirty = 1;
                }
            }
        }
        unlock_sin(sin);
    }
    return ret;
}
//Generate  disk_entry from disk_inode
static int
sfatfs_getdirentry_sub_nolock(struct sfatfs_fs *sfatfs, struct sfatfs_inode *sin, uint16_t offset, struct sfatfs_disk_entry *entry) {
	entry->ino = offset;//index number equals to the offset in file entry table
	//memcpy, not strcpy
	memcpy(entry->name, (sfatfs->dinos + offset)->name, SFATFS_MAX_FNAME_LEN);
	entry->name[SFATFS_MAX_FNAME_LEN] = '\0';
    return 0;
}
/*
 * Used for list the dir. This function is like the FindNextFile() in windows API.
 * Each call returns a file entry in current dir. The offset is saved in iobuf,
 */
static int
sfatfs_getdirentry(struct inode *node, struct iobuf *iob) {
	struct sfatfs_disk_entry *entry;
	if ((entry = kmalloc(sizeof(struct sfatfs_disk_entry))) == NULL) {
		return -E_NO_MEM;
	}
	struct sfatfs_fs *fs = fsop_info(vop_fs(node), sfatfs);
	struct sfatfs_inode *sin = vop_info(node, sfatfs_inode);
	off_t offset = iob->io_offset;
	int ret;

	if (offset < 0 || offset > fs->super.ndirentry) {
		kfree(entry);
		return -E_INVAL;
	}
	if (offset == fs->super.ndirentry){
		kfree(entry);
		return -E_NOENT;
	}
	lock_sin(sin);
	if ((ret = sfatfs_getdirentry_sub_nolock(fs, sin, offset, entry)) != 0) {
		unlock_sin(sin);
		goto out;
	}
	unlock_sin(sin);
	ret = iobuf_move(iob, entry->name, strlen(entry->name) + 1, 1, NULL);
	iob->io_resid = iob->io_len - 1;//TO make the iob->offset ++, points to the next file entry
out:
	kfree(entry);
	return ret;
}
//When refCount == 0, destroy the inode
static int
sfatfs_reclaim(struct inode *node) {
	return 0;//TODO
}
//Get the inode;s type, DIR or REGULAR FILE.
static int
sfatfs_gettype(struct inode *node, uint32_t *type_store) {
    struct sfatfs_disk_inode *din = vop_info(node, sfatfs_inode)->din;
	if (din->attr & SFATFS_TYPE_DIR_MASK)
        *type_store = S_IFDIR;
	else if (din->attr & SFATFS_TYPE_FILE_MASK)
        *type_store = S_IFREG;
	else
    	panic("invalid file attr %d.\n", din->attr);
	return 0;
}
// Truncate file to the pos
static int
sfatfs_tryseek(struct inode *node, off_t pos) {
    if (pos < 0 || pos >= SFATFS_MAX_FILE_SIZE) {
        return -E_INVAL;
    }
    struct sfatfs_inode *sin = vop_info(node, sfatfs_inode);
    if (pos > sin->din->size) {
        return vop_truncate(node, pos);
    }
    return 0;
}
//Truncate file to the given length
static int
sfatfs_truncfile(struct inode *node, off_t len) {
	struct sfatfs_fs *fs = fsop_info(vop_fs(node), sfatfs);
	struct sfatfs_inode *sin = vop_info(node, sfatfs_inode);
	struct sfatfs_disk_inode *din = sin->din;
	int ret = 0;
	if (din->size == len) return 0;
	uint32_t tblks = ROUNDUP_DIV(len, SFATFS_BLKSIZE);	
	uint32_t nblks = ROUNDUP_DIV(din->size, SFATFS_BLKSIZE);
	lock_sin(sin);
	if (nblks < tblks) { //append
		while (nblks != tblks) {
			if ((ret = sfatfs_bmap_load_nolock(fs, sin, nblks, NULL)) != 0){
				goto out_unlock;
			}
			nblks ++;
		}
	} else if (tblks < nblks) { //Truncate
		while (tblks != nblks) {
			if ((ret = sfatfs_bmap_truncate_nolock(fs, sin)) != 0) {
				goto out_unlock;
			}
			nblks --;
		}
	}
	din->size = len;
	sin->dirty = 1;

out_unlock:
	unlock_sin(sin);
    return ret;
}
/* Lookup the path & return the corresponding inode
 * Due to the Flat DIR construction, lookup only need to call the lookup_once function.
 */
static int
sfatfs_lookup(struct inode *node, char *path, struct inode **node_store) {
    struct sfatfs_fs *sfatfs = fsop_info(vop_fs(node), sfatfs);
    assert(*path != '\0' && *path != '/');
    vop_ref_inc(node);
    struct sfatfs_inode *sin = vop_info(node, sfatfs_inode);
    if (!(sin->din->attr & SFATFS_TYPE_DIR_MASK)) {
        vop_ref_dec(node);
        return -E_NOTDIR;
    }
    struct inode *subnode;
    int ret = sfatfs_lookup_once(sfatfs, sin, path, &subnode, NULL);

	vop_ref_dec(node);
    if (ret != 0) {
        return ret;
    }
    *node_store = subnode;
    return 0;
}
//Create a new file by the given filename
static int
sfatfs_create(struct inode *node, const char *name, bool excl, struct inode **node_store){
	//check
	if (strlen(name) > SFATFS_MAX_FNAME_LEN) {
		return -E_TOO_BIG;
	}
	if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) {
		return -E_EXISTS;
	}
	struct sfatfs_fs *fs = fsop_info(vop_fs(node), sfatfs);
	struct sfatfs_inode *sin = vop_info(node, sfatfs_inode);
	int ret;
	if (fs->super.ndirentry >= SFATFS_FILE_NENTRY) {
		return -E_NO_MEM;
	}
	lock_sin(sin);
	{
		uint16_t bno;
		if ((ret = sfatfs_bmap_findfree_nolock(fs, &bno)) != 0) {
			goto out_unlock;
		}
		struct sfatfs_disk_inode *ino = fs->dinos + fs->super.ndirentry;
		memset(ino->name, 0, SFATFS_MAX_FNAME_LEN);
		memcpy(ino->name, name, strlen(name));
		ino->attr = 0x20 | 0x01;
		ino->size = 0;
		ino->first_cluster = bno;
		//meta data
		*(fs->fat + bno) = FAT_EOF;
		fs->super.ndirentry ++;	
		fs->super.unused_blocks --;
		fs->super_dirty = 1;
	}
out_unlock:
	unlock_sin(sin);
	if (ret == 0) {
		struct inode *tnode;
		if ((ret = sfatfs_load_inode(fs, &tnode, fs->super.ndirentry - 1)) == 0) {
			assert(tnode != NULL);
			*node_store = tnode;
		}
	}
	return ret;
}

static const struct inode_ops sfatfs_node_dirops = {
    .vop_magic                      = VOP_MAGIC,
    .vop_open                       = sfatfs_opendir,
    .vop_close                      = sfatfs_close,
    .vop_read                       = NULL_VOP_ISDIR,
    .vop_write                      = NULL_VOP_ISDIR,
    .vop_fstat                      = sfatfs_fstat,
    .vop_fsync                      = sfatfs_fsync,
    .vop_mkdir                      = NULL_VOP_UNIMP,//
    .vop_link                       = NULL_VOP_UNIMP,//
    .vop_rename                     = NULL_VOP_UNIMP,//
    .vop_readlink                   = NULL_VOP_ISDIR,
    .vop_symlink                    = NULL_VOP_UNIMP,
    .vop_namefile                   = NULL_VOP_UNIMP,//
    .vop_getdirentry                = sfatfs_getdirentry,
    .vop_reclaim                    = sfatfs_reclaim,
    .vop_ioctl                      = NULL_VOP_INVAL,
    .vop_gettype                    = sfatfs_gettype,
    .vop_tryseek                    = NULL_VOP_ISDIR,
    .vop_truncate                   = NULL_VOP_ISDIR,
    .vop_create                     = sfatfs_create,
    .vop_unlink                     = NULL_VOP_UNIMP,//
    .vop_lookup                     = sfatfs_lookup,
    .vop_lookup_parent              = NULL_VOP_UNIMP,//
};

static const struct inode_ops sfatfs_node_fileops = {
    .vop_magic                      = VOP_MAGIC,
    .vop_open                       = sfatfs_openfile,
    .vop_close                      = sfatfs_close,
    .vop_read                       = sfatfs_read,
    .vop_write                      = sfatfs_write,
    .vop_fstat                      = sfatfs_fstat,
    .vop_fsync                      = sfatfs_fsync,
    .vop_mkdir                      = NULL_VOP_NOTDIR,
    .vop_link                       = NULL_VOP_NOTDIR,
    .vop_rename                     = NULL_VOP_NOTDIR,
    .vop_readlink                   = NULL_VOP_NOTDIR,
    .vop_symlink                    = NULL_VOP_NOTDIR,
    .vop_namefile                   = NULL_VOP_NOTDIR,
    .vop_getdirentry                = NULL_VOP_NOTDIR,
    .vop_reclaim                    = sfatfs_reclaim,
    .vop_ioctl                      = NULL_VOP_INVAL,
    .vop_gettype                    = sfatfs_gettype,
    .vop_tryseek                    = sfatfs_tryseek,
    .vop_truncate                   = sfatfs_truncfile,
    .vop_create                     = NULL_VOP_NOTDIR,
    .vop_unlink                     = NULL_VOP_NOTDIR,
    .vop_lookup                     = NULL_VOP_NOTDIR,
    .vop_lookup_parent              = NULL_VOP_NOTDIR,
};

