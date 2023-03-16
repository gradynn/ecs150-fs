#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fs.h>

#define ASSERT(cond, func)                               \
do {                                                     \
	if (!(cond)) {                                       \
		fprintf(stderr, "Function '%s' failed\n", func); \
		exit(EXIT_FAILURE);                              \
	} 													 \
	else {											 	 \
		printf("Function '%s' passed\n", func);		 	 \
	}                                                    \  
} while (0)

int main(int argc, char *argv[])
{
	char *diskname;
	int ret; 
	diskname = argv[1];

	//FS_MOUNT()
	ret = fs_mount("not_real_disk.fs");
	ASSERT(ret == -1, "fs_mount error check");

	//FS_INFO()
	ret = fs_info();
	ASSERT(ret == -1, "fs_info error check");

	//FS_CREATE()
	ret = fs_mount(diskname);
	ret = fs_create("aaaaaaaaaaaaaaaaaaaaaaaaa");
	ASSERT(ret == -1, "fs_create filename too long");
	ret = fs_create(NULL);
	ASSERT(ret == -1, "fs_create filename NULL");
	fs_create("my_file");
	ret = fs_create("my_file");
	ASSERT(ret ==-1, "fs_create file already exists");

	//FS_OPEN()
	for ( int i = 0; i < 32; i++)
		fs_open("my_file");
	ret = fs_open("my_file");
	ASSERT(ret == -1, "fs_open max fd table");
	//FS_CLOSE()
	ret = fs_close(-2)
	ASSERT(ret == -1, "fs_close fs out of bounds");
	ret = fs_close(-2)
	ASSERT(ret == -1, "fs_close fs out of bounds");
	//FS_DELETE()

	//FS_UMOUNT()

	return 0; 
}