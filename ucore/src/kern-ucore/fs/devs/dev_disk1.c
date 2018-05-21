#include <types.h>
#include <mmu.h>
#include <slab.h>
#include <sem.h>
#include <ide.h>
#include <inode.h>
#include <dev.h>
#include <vfs.h>
#include <iobuf.h>
#include <error.h>
#include <assert.h>

#define DISK1_BLKSIZE                   PGSIZE
#define DISK1_BUFSIZE                   (4 * DISK1_BLKSIZE)
#define DISK1_BLK_NSECT                 (DISK1_BLKSIZE / SECTSIZE)

static char *disk1_buffer;
static semaphore_t disk1_sem;

static void lock_disk1(void)
{
	down(&(disk1_sem));
}

static void unlock_disk1(void)
{
	up(&(disk1_sem));
}

static int disk1_open(struct device *dev, uint32_t open_flags)
{
	return 0;
}

static int disk1_close(struct device *dev)
{
	return 0;
}

static void disk1_read_blks_nolock(uint32_t blkno, uint32_t nblks)
{
	int ret;
	uint32_t sectno = blkno * DISK1_BLK_NSECT, nsecs =
	    nblks * DISK1_BLK_NSECT;
	if ((ret =
	     ide_read_secs(DISK1_DEV_NO, sectno, disk1_buffer, nsecs)) != 0) {
		panic
		    ("disk1: read blkno = %d (sectno = %d), nblks = %d (nsecs = %d): 0x%08x.\n",
		     blkno, sectno, nblks, nsecs, ret);
	}
}

static void disk1_write_blks_nolock(uint32_t blkno, uint32_t nblks)
{
	int ret;
	uint32_t sectno = blkno * DISK1_BLK_NSECT, nsecs =
	    nblks * DISK1_BLK_NSECT;
	if ((ret =
	     ide_write_secs(DISK1_DEV_NO, sectno, disk1_buffer, nsecs)) != 0) {
		panic
		    ("disk1: write blkno = %d (sectno = %d), nblks = %d (nsecs = %d): 0x%08x.\n",
		     blkno, sectno, nblks, nsecs, ret);
	}
}

static int disk1_io(struct device *dev, struct iobuf *iob, bool write)
{
	off_t offset = iob->io_offset;
	size_t resid = iob->io_resid;
	uint32_t blkno = offset / DISK1_BLKSIZE;
	uint32_t nblks = resid / DISK1_BLKSIZE;

	/* don't allow I/O that isn't block-aligned */
	if ((offset % DISK1_BLKSIZE) != 0 || (resid % DISK1_BLKSIZE) != 0) {
		return -E_INVAL;
	}

	/* don't allow I/O past the end of disk1 */
	if (blkno + nblks > dev->d_blocks) {
		return -E_INVAL;
	}

	/* read/write nothing ? */
	if (nblks == 0) {
		return 0;
	}

	lock_disk1();
	while (resid != 0) {
		size_t copied, alen = DISK1_BUFSIZE;
		if (write) {
			iobuf_move(iob, disk1_buffer, alen, 0, &copied);
			assert(copied != 0 && copied <= resid
			       && copied % DISK1_BLKSIZE == 0);
			nblks = copied / DISK1_BLKSIZE;
			disk1_write_blks_nolock(blkno, nblks);
		} else {
			if (alen > resid) {
				alen = resid;
			}
			nblks = alen / DISK1_BLKSIZE;
			disk1_read_blks_nolock(blkno, nblks);
			iobuf_move(iob, disk1_buffer, alen, 1, &copied);
			assert(copied == alen && copied % DISK1_BLKSIZE == 0);
		}
		resid -= copied, blkno += nblks;
	}
	unlock_disk1();
	return 0;
}

static int disk1_ioctl(struct device *dev, int op, void *data)
{
	return -E_UNIMP;
}

static void disk1_device_init(struct device *dev)
{
	memset(dev, 0, sizeof(*dev));
	static_assert(DISK1_BLKSIZE % SECTSIZE == 0);
	if (!ide_device_valid(DISK1_DEV_NO)) {
		panic("disk1 device isn't available.\n");
	}
	dev->d_blocks = ide_device_size(DISK1_DEV_NO) / DISK1_BLK_NSECT;
	dev->d_blocksize = DISK1_BLKSIZE;
	dev->d_open = disk1_open;
	dev->d_close = disk1_close;
	dev->d_io = disk1_io;
	dev->d_ioctl = disk1_ioctl;
	sem_init(&(disk1_sem), 1);

	static_assert(DISK1_BUFSIZE % DISK1_BLKSIZE == 0);
	if ((disk1_buffer = kmalloc(DISK1_BUFSIZE)) == NULL) {
		panic("disk1 alloc buffer failed.\n");
	}
}

void dev_init_disk1(void)
{
	struct inode *node;
	if ((node = dev_create_inode()) == NULL) {
		panic("disk1: dev_create_node.\n");
	}
	disk1_device_init(vop_info(node, device));

	int ret;
	if ((ret = vfs_add_dev("disk1", node, 1)) != 0) {
		panic("disk1: vfs_add_dev: %e.\n", ret);
	}
}
