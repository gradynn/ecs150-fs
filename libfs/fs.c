#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "disk.h"
#include "fs.h"

// BLOCK SIZE: 4096

/* TODO: Phase 1 */
struct superblock
{
	 //unsigned int
	char signature[8];
	uint16_t total_blk_count;
	uint16_t rdir_blk;
	uint16_t data_blk;
	uint16_t data_blk_count;
	uint8_t  fat_blk_count;
	uint8_t padding[4079];
}__attribute__((packed));

struct file_alloc_table
{
	uint16_t next;
}__attribute__((packed));

struct root_directory
{
	char filename[FS_FILENAME_LEN];
	uint32_t filesize;
	uint16_t blk_index;
	uint8_t padding[10];
}__attribute__((packed));

struct superblock super;
struct file_alloc_table *fat_arr;
struct root_directory root_dir;

int fs_mount(const char *diskname)
{
	int open_disk = block_disk_open(diskname);
	if (open_disk == -1)
		return -1;
	
	int disk_count = block_disk_count();
	if (disk_count == -1)
		return -1;
	
	int read = block_read(0,&super);
	if ( read == -1)
		return -1; 
	
	read = block_read(super.root_idx, &root_dir);
	if (read == -1)
		return -1;
		
	return 0;
}

int fs_umount(void)
{
	/* TODO: Phase 1 */
	//write to disk
}

int fs_info(void)
{
	/* TODO: Phase 1 */
	printf("FS Info:\ntotal_blk_count=%d\n", super.total_blk_count);
	printf("fat_blk_count=%d\n", super.fat_blk_count);
	printf("rdir_blk=%d\n", super.rdir_blk);
	printf("data_blk%d\n", super.data_blk);
	printf("data_blk_count=d\n", super.data_blk_count);
	printf("super.fat_blk = %d\n", super.fat_blk);
}

int fs_create(const char *filename)
{
	/* TODO: Phase 2 */
}

int fs_delete(const char *filename)
{
	/* TODO: Phase 2 */
}

int fs_ls(void)
{
	/* TODO: Phase 2 */
}

int fs_open(const char *filename)
{
	/* TODO: Phase 3 */
}

int fs_close(int fd)
{
	/* TODO: Phase 3 */
}

int fs_stat(int fd)
{
	/* TODO: Phase 3 */
}

int fs_lseek(int fd, size_t offset)
{
	/* TODO: Phase 3 */
}

int fs_write(int fd, void *buf, size_t count)
{
	/* TODO: Phase 4 */
}

int fs_read(int fd, void *buf, size_t count)
{
	/* TODO: Phase 4 */
}

