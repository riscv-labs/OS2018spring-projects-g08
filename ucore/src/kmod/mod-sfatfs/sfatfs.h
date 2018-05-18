#ifndef __KERN_FS_sfatfs_sfatfs_H__
#define __KERN_FS_sfatfs_sfatfs_H__

#include <types.h>
#include <list.h>
#include <sem.h>
#include <unistd.h>

#define SFATFS_MAGIC                                   0x55AA7c1d              /* magic number for sfatfs */
#define SFATFS_BLKSIZE                                 PGSIZE                  /* size of block */
#define SFATFS_MAX_INFO_LEN                            20                      /* max length of infomation */
#define SFATFS_MAX_FNAME_LEN      15							/* max length of filename */
#define SFATFS_MAX_FILE_SIZE                           (1024UL * 1024)   /* max file size (256K) */
#define SFATFS_BLKN_SUPER                              0                       /* block the superblock lives in */
#define SFATFS_RECORD_ROOT                               0                       /* location of the root dir inode */
#define SFATFS_BLKN_FAT                            1                       /* 1st block of the freemap */
#define SFATFS_BLKN_ROOT	0 /* location of the root dir block */	

/* # of bits in a block */
#define SFATFS_BLKBITS                                 (SFATFS_BLKSIZE * CHAR_BIT)

/* # of entries in a block */
#define SFATFS_BLK_NENTRY                              (SFATFS_BLKSIZE / sizeof(uint16_t))

#define SFATFS_ROOTDIR_OFFSET 	512			/* Offset in bytes for the file entries in the block SFATFS_BLKN_ROOT */

#define SFATFS_FILE_NENTRY ((SFATFS_BLKSIZE - SFATFS_ROOTDIR_OFFSET) / sizeof(struct sfatfs_disk_inode))			/* Max number of file entries */

/* file types */
#define SFATFS_TYPE_INVAL                              0       /* Should not appear on disk */
#define SFATFS_TYPE_FILE                               1
#define SFATFS_TYPE_DIR                                2
#define SFATFS_TYPE_LINK                               3

#define SFATFS_TYPE_FILE_MASK	0x20
#define SFATFS_TYPE_DIR_MASK	0x10

#define SFATFS_HLIST_SHIFT                             10
#define SFATFS_HLIST_SIZE                              (1 << SFATFS_HLIST_SHIFT)
#define sfatin_hashfn(x)                               (hash32(x, SFATFS_HLIST_SHIFT))

#define FAT_FREE 0x0000				/* FAT MARK, FREE */
#define FAT_RESERVE 0x0001			/* FAT MARK, reserved, used but not data */
#define FAT_EOF 0xffff				/* FAT MARK, the fat link is ended */

/*
 * On-disk superblock
 */
struct sfatfs_super {
    uint32_t magic;                                 /* magic number, should be sfatfs_MAGIC */
    uint32_t blocks;                                /* # of blocks in fs */
    uint32_t unused_blocks;                         /* # of unused blocks in fs */
	uint16_t ndirentry;		/* number of entrys in root dir */
    char info[SFATFS_MAX_INFO_LEN];                /* infomation for sfatfs  */
};

/* file entry (on disk) */
struct sfatfs_disk_inode {
    char name[SFATFS_MAX_FNAME_LEN];               /* file name, no ending '\0' */
	uint8_t attr;
	uint16_t first_cluster;
	uint32_t size;
};


/* file entry for program use */
struct sfatfs_disk_entry {
    uint16_t ino;                                   /* inode number, file entry number */
    char name[SFATFS_MAX_FNAME_LEN + 1];               /* file name with ending '\0'*/
};

#define sfatfs_dentry_size                             \
    sizeof(((struct sfatfs_disk_entry *)0)->name)

/* inode for sfatfs */
struct sfatfs_inode {
    struct sfatfs_disk_inode *din;                     /* on-disk inode */
    uint16_t ino;                                   /* inode number */
    bool dirty;                                     /* true if inode modified */
    int reclaim_count;                              /* kill inode if it hits zero */
    semaphore_t sem;                                /* semaphore for din */
    list_entry_t inode_link;                        /* entry for linked-list in sfatfs_fs */
    list_entry_t hash_link;                         /* entry for hash linked-list in sfatfs_fs */
};

#define le2sfatin(le, member)                          \
    to_struct((le), struct sfatfs_inode, member)

/* filesystem for sfatfs */
struct sfatfs_fs {
    struct sfatfs_super super;                         /* on-disk superblock */
    struct device *dev;                             /* device mounted on */
    uint16_t *fat;                         /* the whole FAT */
	struct sfatfs_disk_inode *dinos;		/* All the disk inodes */ 
    bool super_dirty;                               /* true if super/freemap modified */
    void *sfatfs_buffer;                               /* buffer for non-block aligned io */
    semaphore_t fs_sem;                             /* semaphore for fs */
    semaphore_t io_sem;                             /* semaphore for io */
    semaphore_t mutex_sem;                          /* semaphore for link/unlink and rename */
    list_entry_t inode_list;                        /* inode linked-list */
    list_entry_t *hash_list;                        /* inode hash linked-list */
};

struct fs;
struct inode;

void sfatfs_init(void);
int sfatfs_mount(const char *devname);

void lock_sfatfs_fs(struct sfatfs_fs *sfatfs);
void lock_sfatfs_io(struct sfatfs_fs *sfatfs);
void lock_sfatfs_mutex(struct sfatfs_fs *sfatfs);
void unlock_sfatfs_fs(struct sfatfs_fs *sfatfs);
void unlock_sfatfs_io(struct sfatfs_fs *sfatfs);
void unlock_sfatfs_mutex(struct sfatfs_fs *sfatfs);

int sfatfs_rblock(struct sfatfs_fs *sfatfs, void *buf, uint32_t blkno, uint32_t nblks);
int sfatfs_wblock(struct sfatfs_fs *sfatfs, void *buf, uint32_t blkno, uint32_t nblks);
int sfatfs_rbuf(struct sfatfs_fs *sfatfs, void *buf, size_t len, uint32_t blkno, off_t offset);
int sfatfs_wbuf(struct sfatfs_fs *sfatfs, void *buf, size_t len, uint32_t blkno, off_t offset);
int sfatfs_sync_super(struct sfatfs_fs *sfatfs);
int sfatfs_sync_freemap(struct sfatfs_fs *sfatfs);
int sfatfs_clear_block(struct sfatfs_fs *sfatfs, uint32_t blkno, uint32_t nblks);

int sfatfs_load_inode(struct sfatfs_fs *sfatfs, struct inode **node_store, uint32_t ino);

#endif /* !__KERN_FS_sfatfs_sfatfs_H__ */

