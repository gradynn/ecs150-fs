/*How to check is disk is mounted?
Does phase 3 look correct? */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "disk.h"
#include "fs.h"

#define FAT_EOC 0xFFFF
#define SIGN_LEN 8
#define BLOCK_SIZE 4096

/* TODO: Phase 1 */
struct superblock
{
	char signature[SIGN_LEN];
	uint16_t total_blk_count;
	uint16_t rdir_blk;
	uint16_t data_blk;
	uint16_t data_blk_count;
	uint8_t  fat_blk_count;
	uint8_t padding[4079];
}__attribute__((packed));

struct file
{
	char filename[FS_FILENAME_LEN];
	uint32_t filesize;
	uint16_t blk_index;
	uint8_t padding[10];
}__attribute__((packed));

struct file_descriptor
{
	int rdir_index;
	size_t offset;
};


struct superblock super;
uint16_t *fat_arr;
struct file root_dir[FS_FILE_MAX_COUNT];
struct file_descriptor fd_table[32];
int disk_mounted;

int fs_mount(const char *diskname)
{
	disk_mounted = 0;
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
	for (int i = 0; i < SIGN_LEN; i++)
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
	int extra = super.data_blk_count % BLOCK_SIZE;
	if (extra > 0 )
		fat_arr = malloc(sizeof(uint16_t) * BLOCK_SIZE * (super.data_blk_count/BLOCK_SIZE +1));
	else 
		fat_arr = malloc(sizeof(uint16_t) * super.data_blk_count);
	
	// copy FAT from disk to memory
	for (int i = 1; i <= super.fat_blk_count; i++)
	{	
		if (block_read(i, &fat_arr[(i - 1) * (BLOCK_SIZE / 2)]) != 0)
			return -1;
	}

	// initialize file descriptor table
	for (int i = 0; i < FS_OPEN_MAX_COUNT; i++)
		fd_table[i].rdir_index= -1;

	disk_mounted = 1;

	return 0;
}

int fs_umount(void)
{
	if (disk_mounted == 0)
		return -1;

	//check if file descriptors are still open
	for (int i = 0; i < FS_OPEN_MAX_COUNT; i++)
	{
		if (fd_table[i].rdir_index != -1)
			return -1;
	}
	// write super block to disk
	if (block_write(0, &super) != 0)
		return -1;

	// write fat to disk
	for (int i = 1; i <= super.fat_blk_count; i++)
	{
		if(block_write(i, &fat_arr[(i - 1) * (BLOCK_SIZE / 2)]) != 0)
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

	disk_mounted = 0;

	return 0; 
}

int fs_info(void)
{
	if (disk_mounted == 0)
		return -1;

	printf("FS Info:\ntotal_blk_count=%d\n", super.total_blk_count);
	printf("fat_blk_count=%d\n", super.fat_blk_count);
	printf("rdir_blk=%d\n", super.rdir_blk);
	printf("data_blk=%d\n", super.data_blk);
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
	
	return 0;
}

int fs_create(const char *filename)
{
	if ( disk_mounted == 0)
		return -1; 
	if (filename == NULL || strlen(filename) > FS_FILENAME_LEN)
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
			
			new_file.blk_index = FAT_EOC;

			root_dir[i] = new_file;
			return 0;
		}
	}

	return -1;	
}

int fs_delete(const char *filename)
{
	if (disk_mounted == 0)
		return -1;
	if (filename == NULL || strlen(filename) > FS_FILENAME_LEN)
		return -1; 
	
	// check if the file is open in fd_table
	for (int i = 0; i < FS_OPEN_MAX_COUNT; i++)
	{
		if ( fd_table[i].rdir_index != -1 &&
			strcmp(root_dir[fd_table[i].rdir_index].filename, filename) == 0)
			return -1;
	}
	
	// find file in root directory
	for (int i = 0; i < FS_FILE_MAX_COUNT; i++)
	{
		if (strcmp(root_dir[i].filename, filename) == 0)
		{
			// free FAT blocks
			int temp = root_dir[i].blk_index;
			while (fat_arr[temp] != FAT_EOC)
			{
				int next = fat_arr[temp];
				fat_arr[temp] = 0;
				temp = next;
			}
			fat_arr[temp] = 0;

			// free root directory entry
			strcpy(root_dir[i].filename, "\0");
			root_dir[i].filesize = 0;
			root_dir[i].blk_index = 0;
			return 0;
		}
	}
	return -1; 
}

int fs_ls(void)
{
	if (disk_mounted == 0)
		return -1;
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

	return 0;
}

int fs_open(const char *filename)
{
	if (disk_mounted == 0)
		return -1;
	if (filename == NULL || strlen(filename) > FS_FILENAME_LEN)
		return -1; 
		
	// find file in root directory
	for (int i = 0; i < FS_FILE_MAX_COUNT; i++)
	{
		if ( strcmp(root_dir[i].filename, filename) == 0)
		{
			for(int j = 0; j <FS_OPEN_MAX_COUNT; j++)
			{
				if ( fd_table[j].rdir_index == -1)
				{
					fd_table[j].rdir_index = i;
					fd_table[j].offset = 0;
					return j;
				}
			}
		}
	}
	return -1; 
}

int fs_close(int fd)
{
	if (disk_mounted == 0)
		return -1;

	//fd out of bounds
	if (fd >31 || fd < 0)
		return -1; 
	//fd is not currently open
	if (fd_table[fd].rdir_index == -1)
		return -1;
	
	fd_table[fd].rdir_index = -1;
	fd_table[fd].offset = 0;
	
	return 0;
}

int fs_stat(int fd)
{
	if (disk_mounted == 0)
		return -1;

	//fd out of bounds
	if (fd > 31 || fd < 0)
		return -1;
	//fd is not currently open
	if (fd_table[fd].rdir_index == -1)
		return -1;

	return (root_dir[fd_table[fd].rdir_index].filesize);
}

int fs_lseek(int fd, size_t offset)
{
	if (disk_mounted == 0)
		return -1;

	//fd out of bounds
	if (fd > 31 || fd < 0)
		return -1;
	//fd is not currently open
	if (fd_table[fd].rdir_index == -1)
		return -1;
	//offset if bigger than the size of the file
	if (offset > root_dir[fd_table[fd].rdir_index].filesize)
		return -1;

	fd_table[fd].offset = offset;

	return 0;
}

int read_get_index(int fd)
{
	int offset = fd_table[fd].offset;
	int count = offset / BLOCK_SIZE;
	int index = root_dir[fd_table[fd].rdir_index].blk_index;
	
	for (int i = 0; i < count; i++)
	{
		index = fat_arr[index];
	}

	return index;
}

int blk_alloc(int fd)
{
	int index = root_dir[fd_table[fd].rdir_index].blk_index;

	// find first available block in FAT
	for (int i = 0; i < super.data_blk_count; i++)
	{
		if (fat_arr[i] == 0)
		{
			if (index == FAT_EOC) 
			{
				root_dir[fd_table[fd].rdir_index].blk_index = i;
				fat_arr[i] = FAT_EOC;
				return i;
			}

			while (fat_arr[index] != FAT_EOC)
			{
			index = fat_arr[index];
			}

			fat_arr[index] = i;
			fat_arr[i] = FAT_EOC;
			return i;
		}
	}

	return -1;
}

int fs_write(int fd, void *buf, size_t count)
{
	if (disk_mounted == 0)
		return -1; 

	//fd out of bounds
	if (fd > 31 || fd < 0)
		return -1;
	//fd is not currently open
	if (fd_table[fd].rdir_index == -1)
		return -1;
	if ( buf == NULL)
		return -1;
	
	size_t offset = fd_table[fd].offset % BLOCK_SIZE;
	int data_idx = read_get_index(fd);
	int total_blks = (count+offset)/ BLOCK_SIZE;
	if ( (count+offset)%BLOCK_SIZE > 0) //if we don't read exact multiple of block size we need to read extra block
		total_blks += 1; 

	int buf_idx = 0;
	uint8_t *temp_buf = (uint8_t*)buf; 
	for (int i = 0; i < total_blks; i++)
	{
		if (data_idx == FAT_EOC)//alloc new block
		{
			data_idx = blk_alloc(fd);
			if(data_idx == -1)
				return buf_idx; 
		}
		if ( offset == 0 && count >= BLOCK_SIZE)//full read
		{
			if (block_write(data_idx + super.data_blk, &temp_buf[buf_idx]) != 0)
		 		return -1;
			buf_idx += BLOCK_SIZE;
			count -= BLOCK_SIZE;
		}
		else //partial read
		{
			uint8_t bounce[BLOCK_SIZE];
			if ( block_read(data_idx + super.data_blk, bounce) != 0) //read full block first
				return -1; 

			int bytes_write = 0; 
			if ( i == total_blks -1) // partial
				bytes_write= count;
			else //full
				bytes_write = BLOCK_SIZE - offset;

			memcpy(&bounce[offset], &temp_buf[buf_idx], bytes_write);
			buf_idx += bytes_write;
			count -= bytes_write;
			offset = 0;
			
			if (block_write(data_idx + super.data_blk, bounce) != 0)
				return -1;
		}
		data_idx = fat_arr[data_idx];
	}
	fd_table[fd].offset = fd_table[fd].offset + buf_idx;
	root_dir[fd_table[fd].rdir_index].filesize = root_dir[fd_table[fd].rdir_index].filesize - offset + buf_idx;
	return buf_idx;
}

int fs_read(int fd, void *buf, size_t count)
{
	if (disk_mounted == 0)
		return -1; 
		
	//fd out of bounds
	if (fd > 31 || fd < 0)
		return -1;
	//fd is not currently open
	if (fd_table[fd].rdir_index == -1)
		return -1;
	if ( buf == NULL)
		return -1;
	
	size_t offset = fd_table[fd].offset % BLOCK_SIZE;
	int data_idx = read_get_index(fd); //starting data block to read from
	int total_blks = (count+offset)/ BLOCK_SIZE;
	if ( (count+offset)%BLOCK_SIZE > 0) //if we don't read exact multiple of block size we need to read extra block
		total_blks += 1; 

	int buf_idx = 0; 
	uint8_t *temp_buf = malloc(sizeof(uint8_t) * count);
	for ( int i = 0; i < total_blks; i++)
	{
		if (data_idx == FAT_EOC)
			break;
		if ( offset == 0 && count >= BLOCK_SIZE)//full read
		{
			if (block_read(data_idx + super.data_blk, &temp_buf[buf_idx]) != 0)
		 		return -1;
			buf_idx += BLOCK_SIZE;
			count -= BLOCK_SIZE;
		}
		else //partial read
		{
			uint8_t bounce[BLOCK_SIZE];
			if ( block_read(data_idx + super.data_blk, bounce) != 0)
				return -1; 
			int bytes_read = 0; 
			if ( i == total_blks -1)
				bytes_read = count;
			else
				bytes_read = BLOCK_SIZE - offset;


			memcpy(&temp_buf[buf_idx], &bounce[offset], bytes_read);
			buf_idx += bytes_read;
			count -= bytes_read;
			offset = 0;
		}
		data_idx = fat_arr[data_idx];
	}
	memcpy(buf, (void*)temp_buf, buf_idx);
	free(temp_buf);
	fd_table[fd].offset = fd_table[fd].offset + buf_idx;
	return buf_idx; 
}
