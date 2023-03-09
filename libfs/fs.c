/*rdir_free_ratio => what does that mean? Do we mount only one root dir?
Do we write to the disk in fs_umount()?
Setting up file - FAT_EOC, FAT allocation 
How does signature get added to a disk?*/
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "disk.h"
#include "fs.h"

#define FAT_EOC 0xFFFF
// BLOCK SIZE: 4096

/* TODO: Phase 1 */
struct superblock
{
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

/*
struct root_directory
{
	char filename[FS_FILENAME_LEN];
	uint32_t filesize;
	uint16_t blk_index;
	uint8_t padding[10];
}__attribute__((packed));
*/

struct file
{
	char filename[FS_FILENAME_LEN];
	uint32_t filesize;
	uint16_t blk_index;
	uint8_t padding[10];
}__attribute__((packed));

struct superblock super;
uint16_t *fat_arr;
struct file root_dir[FS_FILE_MAX_COUNT];

int fs_mount(const char *diskname)
{
	int open_disk = block_disk_open(diskname);
	if (open_disk == -1)
		return -1;
	
	int disk_count = block_disk_count();
	if (disk_count == -1)
		return -1;
	
	int read = block_read(0, &super);
	if (read == -1)
		return -1;
	
	char *sig = "ECS150FS";
	for (int i = 0; i < 8; i++)
	{
		if (super.signature[i] != sig[i])
			return -1;
	}

	if(super.total_blk_count != disk_count)
		return -1; 
	
	read = block_read(super.rdir_blk, &root_dir);
	if (read == -1)
		return -1;

	// allocate memory for FAT
	fat_arr = malloc(sizeof(uint16_t) * super.data_blk_count);
	
	// copy FAT from disk to memory
	for (int i = 1; i <= super.fat_blk_count; i++)
	{
		if (block_read(i, &fat_arr[i * ((i - 1) * (BLOCK_SIZE / 16))]) != 0)
			return -1;
	}

	return 0;
}

int fs_umount(void)
{
	// write super block to disk
	if (block_write(0, &super) != 0)
		return -1;

	// write fat to disk
	for (int i = 1; i <= super.fat_blk_count; i++)
	{
		if(block_write(i, &fat_arr[i * ((i - 1) * (BLOCK_SIZE / 16))]) != 0)
			return -1;
	}

	// write root directory to disk
	if (block_write(super.rdir_blk, &root_dir) != 0)
		return -1;

	// close currently open disk
	if(block_disk_close() != 0) 
		return -1;

	// free FAT memory
	free(fat_arr);

	return 0; 
}

int fs_info(void)
{
	/* TODO: Phase 1 */
	printf("FS Info:\ntotal_blk_count=%d\n", super.total_blk_count);
	printf("fat_blk_count=%d\n", super.fat_blk_count);
	printf("rdir_blk=%d\n", super.rdir_blk);
	printf("data_blk%d\n", super.data_blk);
	printf("data_blk_count=%d\n", super.data_blk_count);

	// count free blocks in FAT
	int fat_free = 0;
	for (int i = 0; i < super.data_blk_count; i++)
	{
		if (fat_arr[i] == 0)
			fat_free++;
	}
	printf("fat_free_ratio=%d/%d\n", fat_free, super.data_blk_count);

	// count free blocks in root directory
	int rdir_free = 0;
	for (int i = 0; i < FS_FILE_MAX_COUNT; i++)
	{
		if (root_dir[i].filename[0] == '\0')
			rdir_free++;
	}
	printf("rdir_free_ratio=%d/%d\n", rdir_free, FS_FILE_MAX_COUNT);
}

int fs_create(const char *filename)
{
	if (strlen(filename) >= FS_FILENAME_LEN)
		return -1; 

	// find next open space in root directory array
	for (int i = 0; i < FS_FILE_MAX_COUNT; i++)
	{
		if (strcmp(root_dir[i].filename, filename) == 0) 
			return -1;

		if (root_dir[i].filename[0] == '\0')
		{
			struct file new_file;
			strcpy(new_file.filename, filename);
			new_file.filesize = 0;
			
			// find next open space in FAT
			for (int j = 0; j < super.data_blk_count; j++)
			{
				if (fat_arr[j] == 0)
				{
					new_file.blk_index = j;
					fat_arr[j] = FAT_EOC;
					break;
				}
			}

			root_dir[i] = new_file;
			return 0;
		}
	}
	return -1;	
}

int fs_delete(const char *filename)
{
	// find file in root directory
	for (int i = 0; i < FS_FILE_MAX_COUNT; i++)
	{
		if (strcmp(root_dir[i].filename, filename) == 0)
		{
			// free FAT blocks
			int next = root_dir[i].blk_index;
			while (fat_arr[next] != FAT_EOC)
			{
				int temp = fat_arr[next];
				fat_arr[next] = 0;
				next = temp;
			}
			fat_arr[next] = 0;

			// free root directory entry
			strcpy(root_dir[i].filename, "\0");
			root_dir[i].filesize = 0;
			root_dir[i].blk_index = 0;
			return 0;
		}
	}
}

int fs_ls(void)
{
	printf("FS Ls:\n");
	for (int i = 0; i < FS_FILE_MAX_COUNT; i++)
	{
		if (strcmp(root_dir[i].filename, "\0") != 0) {
			printf("file: %s, size: %d, data_blk: %d\n", 
				root_dir[i].filename, 
				root_dir[i].filesize,
				root_dir[i].blk_index);
		}
	}
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

