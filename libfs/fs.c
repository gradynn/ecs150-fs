#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "disk.h"
#include "fs.h"

// BLOCK SIZE: 4096

/* TODO: Phase 1 */
struct disk_meta {
	uint32_t superblock;
	uint32_t FAT;
	uint32_t root_dir;
}__attribute__((packed));

/*
struct superblock {
	uint16_t address;
}

struct FAT {
	uint16_t address;
}

struct root_dir {
	uint16_t address;
}
*/

int fs_mount(const char *diskname)
{
	/* TODO: Phase 1 */
	int open_disk = block_disk_open(diskname);
	if (open_disk == -1)
		return -1;

	struct disk_meta *disk_meta = malloc(sizeof(struct disk_meta));
	disk_meta->superblock = 0;
	

	return 0;
}

int fs_umount(void)
{
	/* TODO: Phase 1 */
}

int fs_info(void)
{
	/* TODO: Phase 1 */
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

