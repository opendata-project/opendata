
#include "cfs.h"

#include <string.h>
#include <iostream>
#include "rocksdb/db.h"
#include "rocksdb/options.h"
#include "rocksdb/slice.h"



Cfs g_cfs_instance;


static const char *hello_str = "Hello World!\n";		//just for demo
static const char *hello_name = "hello";				//just for demo
std::string kDBPath = "/opt/tmp/rocksdb_example";

Cfs::Cfs() {
}

Cfs::~Cfs() {
}

void Cfs::Init() {
	meta_.Init();
}

void Cfs::Finish() {
	meta_.Finish();
}

int Cfs::CfsGetattr(uint64_t ino, struct InodeAttr *inoattr) {
	
	//TODO: later here will add some meta_cache search

	return meta_.GetInodeAttr(ino, inoattr);

}

int Cfs::CfsGetattrTest(uint64_t ino, struct stat *stbuf) {
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


int Cfs::CfsLookup(uint64_t parent, const char *name, uint64_t *inode, struct InodeAttr *inoattr) {
	
	//TODO: later here will add some meta_cache search

	return meta_.Lookup(parent, name, inode, inoattr);

}

int Cfs::CfsReaddir(uint64_t pino, int off, std::vector<struct DirEntry*> *dentries, 
                    std::vector<struct InodeAttr*> *inodes) {
	return 0;
}


void Cfs::CfsRead() {
	using namespace rocksdb;
	DB *db;
	Options options;
	options.IncreaseParallelism();
 
	options.create_if_missing = true;
 
	//open DB
	Status s=DB::Open(options,kDBPath,&db);
	assert(s.ok());
 
	//Put key-value
	s = db->Put(WriteOptions(),"key1","value");
	assert(s.ok());
 
	std::string value;
	//get value
	s = db->Get(ReadOptions(),"key1",&value);
	assert(s.ok());
	std::cout << value << std::endl;

	struct _st {
		char b[20];
	} _internal;
	memset(&_internal, 0, sizeof(_internal));
    std::string tmp;
    s = db->Get(ReadOptions(), Slice("key1"), &tmp);
    if (s.ok()) {
        memcpy(&_internal, tmp.data(), sizeof(char) * tmp.size());
    } else {
        std::cout << s.ToString() << std::endl;
    }
}



