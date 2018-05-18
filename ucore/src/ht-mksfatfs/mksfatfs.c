#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <assert.h>

#define __error(msg, quit, ...)                                                         \
    do {                                                                                \
        fprintf(stderr, #msg ": function %s - line %d: ", __FUNCTION__, __LINE__);      \
        if (errno != 0) {                                                               \
            fprintf(stderr, "[error] %s: ", strerror(errno));                           \
        }                                                                               \
        fprintf(stderr, "\n\t"), fprintf(stderr, __VA_ARGS__);                          \
        errno = 0;                                                                      \
        if (quit) {                                                                     \
            exit(-1);                                                                   \
        }                                                                               \
    } while (0)

#define warn(...)           __error(warn, 0, __VA_ARGS__)
#define bug(...)            __error(bug, 1, __VA_ARGS__)

#define EMPTY_CLUSTER	0x0000
#define RESERVE_CLUSTER 0x0001
#define EOF_CLUSTER	0xffff

#define CLUSTER_SIZE 4096
#define MAX_CLUSTER 2048
#define SECTOR_SIZE 512
#define FILE_NAME_LEN 15
#define MAX_INFO_LEN 20
#define FAT_RECORD_LEN 2 //16 bit
#define FAT_MAX_RECORD 2048
#define SFATFS_MAGIC 0x55AA7c1d

char buf[CLUSTER_SIZE];
char sec[SECTOR_SIZE];

struct dir_record {
	char name[FILE_NAME_LEN];
	uint8_t attr;
	uint16_t first_cluster;
	uint32_t size;
};

struct sfatfs {
	struct {
		uint32_t magic;
		uint32_t blocks;
		uint32_t unused_blocks;
		char info[MAX_INFO_LEN + 1];
	} super;
	uint16_t fat[FAT_MAX_RECORD];
	void *rootdir;
	uint16_t  ndirentry;
	int imgfd;
	uint16_t ninos, next_ino;
};

void safe_chdir(int fd){
	if (fchdir(fd) != 0){
		bug("fchdir failed %d.\n", fd);
	}
}

struct stat *safe_fstat(int fd){
	static struct stat __stat;
	if (fstat(fd, &__stat) != 0) {
		bug("fstat %d failed.\n", fd);
	} 
	return &__stat;
}

struct stat *safe_lstat(const char *name){
	static struct stat __stat;
	if (lstat(name, &__stat) != 0){
		bug("lstat '%s' failed.\n", name);
	}
	return &__stat;
}

void *safe_malloc(size_t size){
	void *ret;
	if ((ret = malloc(size)) == NULL){
		bug("malloc %lu bytes failed.\n", (long unsigned) size);
	}
	return ret;
}

struct sfatfs *create_fs(int imgfd){
	uint32_t ninos, next_ino;
	struct stat *stat = safe_fstat(imgfd);
	if ((ninos = stat->st_size / CLUSTER_SIZE) > MAX_CLUSTER) {
		ninos = MAX_CLUSTER;
		warn("img file is too big (%llu bytes, only use %u blocks).\n",stat->st_size, ninos);
	}
	if ((next_ino = 2) > ninos){
		bug("img file is too small\n");
	}
	struct sfatfs *fs = (struct sfatfs *)safe_malloc(sizeof(struct sfatfs));
	fs->super.magic = SFATFS_MAGIC;
	fs->super.blocks = ninos;
	fs->super.unused_blocks = ninos - next_ino;
	fs->ninos = ninos, fs->next_ino = next_ino, fs->imgfd = imgfd;
	fs->ndirentry = 0;
	snprintf(fs->super.info, MAX_INFO_LEN, "simple linked fs");
	memset(fs->fat, 0, sizeof(fs->fat));
	fs->fat[0] = fs->fat[1] = RESERVE_CLUSTER;

	int tsize = sizeof(struct dir_record);
	int tnum = CLUSTER_SIZE / 2 / tsize;
	tnum *= tsize;
	fs->rootdir = safe_malloc(tnum);
	return fs;
}

void write_fs(struct sfatfs *fs){
	memset(buf, 0, sizeof(buf));
	char *curr = buf;

//	curr = buf + SECTOR_SIZE;
	*((uint32_t *)curr) = fs->super.magic;
	curr += 4;
	*((uint32_t *)curr) = fs->super.blocks;
	curr += 4;
	*((uint32_t *)curr) = fs->super.unused_blocks;
	curr += 4;
	*((uint16_t *)curr) = fs->ndirentry;
	curr += 2;
	memcpy(curr, fs->super.info, MAX_INFO_LEN);
	curr += MAX_INFO_LEN;
	
	curr = buf + SECTOR_SIZE;
	memcpy(curr, fs->rootdir, fs->ndirentry * sizeof(struct dir_record));
	int ret;
	ret = pwrite(fs->imgfd, buf, sizeof(buf), 0);
	ret = pwrite(fs->imgfd, fs->fat, sizeof(fs->fat), 1 * CLUSTER_SIZE);
}

void add_file(struct sfatfs *fs, int fd, const char *filename){
	printf("Add file %s\n", filename);
	uint16_t stb = fs->next_ino;
	uint32_t filesize = 0;
	ssize_t ret;
	if (fs->ndirentry * sizeof(struct dir_record) > SECTOR_SIZE * 3) {
		bug("too many files!\n");
	}
	while ((ret = read(fd, buf, sizeof(buf))) != 0) {
		filesize += ret;
		if (fs->next_ino >= fs->ninos) {
			bug("imgfile is too small, no free space!\n");
		}
		ret = pwrite(fs->imgfd, buf, ret, fs->next_ino * CLUSTER_SIZE);
		fs->next_ino ++;
		fs->super.unused_blocks --;
	}
	if (filesize == 0) fs->next_ino ++; //remain at least one block for the empty file
	uint16_t p = stb;
	while (p < fs->next_ino - 1) {
		fs->fat[p] = p + 1;
		p ++;
	}
	fs->fat[p] = EOF_CLUSTER;
	struct dir_record *rp = (struct dir_record *)fs->rootdir + fs->ndirentry;
	fs->ndirentry ++;
	rp->first_cluster = stb;
	rp->size = filesize;
	rp->attr = 0x0 | 0x20 | 0x01;
	memset(rp->name, 0, sizeof(rp->name));
	memcpy(rp->name, filename, strlen(filename));
}

void open_dir(struct sfatfs *fs){
	DIR *dir;
	if ((dir = opendir(".")) == NULL) {
		bug("opendir failed.\n");
	}

	struct dir_record *rp = fs->rootdir;
	rp->first_cluster = 0;
	rp->size = 0;
	rp->attr = 0x0 | 0x10 | 0x02;
	memset(rp->name, 0, sizeof(rp->name));
	rp->name[0] = '.';
	rp ++;
	rp->first_cluster = 0;
	rp->size = 0;
	rp->attr = 0x0 | 0x10 | 0x02;
	memset(rp->name, 0, sizeof(rp->name));
	rp->name[0] = rp->name[1] =  '.';
	
	fs->ndirentry += 2;

	struct dirent *direntp;
	while ((direntp = readdir(dir)) != NULL) {
		const char *name = direntp->d_name;
		if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) {
			continue;
		}
		if (name[0] == '.') {
			continue;
		}
		if (strlen(name) > FILE_NAME_LEN) {
			bug("file name is too long: %s\n", name);
		}
		struct stat *stat = safe_lstat(name);
		//if (S_ISLNK(stat->st_mode)) 
		if (S_ISREG(stat->st_mode)){
			int fd;
			if ((fd = open(name, O_RDONLY)) < 0) {
				bug("open failed: %s\n", name);
			}
			add_file(fs, fd, name);
			close(fd);
		}
	}
	closedir(dir);
}

int open_img(const char *imgname){
	const char *expect = ".img", *ext = imgname + strlen(imgname) - strlen(expect);
	if (ext <= imgname || strcmp(ext, expect) != 0) {
		bug("invaild .img file name '%s'.\n", imgname);
	}
	int imgfd;
	if ((imgfd = open(imgname, O_WRONLY)) < 0){
		bug("open '%s' failed.\n", imgname);
	}
	return imgfd;
}

int create_img(const char *imgname, const char *home){
	int imgfd = open_img(imgname);
	struct sfatfs *fs = create_fs(imgfd);
	int curfd, homefd;
	if ((curfd = open(".", O_RDONLY)) < 0) {
		bug("get cwd failed.\n");
	}
	if ((homefd = open(home, O_RDONLY | O_NOFOLLOW)) < 0){
		bug("open home directory '%s' failed.\n", home);
	}
	safe_chdir(homefd);
	open_dir(fs);
	write_fs(fs);
	close(imgfd);
	safe_chdir(curfd);
	close(curfd);close(homefd);

	printf("Number of unused blocks: %d\n", fs->super.unused_blocks);
	return 0;
}
int main(int argc, char **argv){
	if (argc != 3) {
		bug("Usage: <input *.img> <input dirname>\n");
	}
	const char *imgname = argv[1], *home = argv[2];
	printf("home dir=%s\n", home);
	if (create_img(imgname, home) != 0) {
		bug("create img failed.\n");
	}
	printf("create %s (%s) successfully.\n", imgname, home);
	return 0;
}
