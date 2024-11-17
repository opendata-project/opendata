
#include <string.h>
#include "cfs.h"


Cfs g_cfs_instance;


static const char *hello_str = "Hello World!\n";		//just for demo
static const char *hello_name = "hello";				//just for demo

Cfs::Cfs()
{
}

Cfs::~Cfs()
{
}

int Cfs::CfsGetattr(uint64_t ino, struct stat *stbuf)
{
	stbuf->st_ino = ino;
	switch (ino) {
	case 1:
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
		break;

	case 2:
		stbuf->st_mode = S_IFREG | 0444;
		stbuf->st_nlink = 1;
		stbuf->st_size = strlen(hello_str);
		break;

	default:
		return -1;
	}
	return 0;
}
