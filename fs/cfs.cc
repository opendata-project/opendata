
#include "cfs.h"

#include <string.h>
#include <iostream>
#include <fcntl.h>
#include "rocksdb/db.h"
#include "rocksdb/options.h"
#include "rocksdb/slice.h"
#include "defines.h"
#include "../util/log.h"



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
	data_.Init();
}

void Cfs::Finish() {
	meta_.Finish();
}

int Cfs::CfsGetattr(uint64_t ino, struct InodeAttr *inoattr) {
	
	//TODO: later here will add some meta_cache search

	return meta_.GetInodeAttr(ino, inoattr);

}

int Cfs::CfsSetattr(uint64_t ino, struct InodeAttr *inoattr, int setmask, struct InodeAttr *new_inoattr) {
	//struct InodeAttr oldattr = {0};
	CfsGetattr(ino, new_inoattr);
	if (setmask & CFS_SET_ATTR_MODE) {
		new_inoattr->mode = inoattr->mode;
	}
	if (setmask & CFS_SET_ATTR_UID) {
		new_inoattr->uid = inoattr->uid;
	}
	if (setmask & CFS_SET_ATTR_GID) {
		new_inoattr->gid = inoattr->gid;
	}
	if (setmask & CFS_SET_ATTR_ATIME) {
		new_inoattr->atime = inoattr->atime;
	}
	if (setmask & CFS_SET_ATTR_MTIME) {
		new_inoattr->mtime = inoattr->mtime;
	}
	struct timespec curtime;
	clock_gettime(CLOCK_REALTIME, &curtime);
	if (setmask & CFS_SET_ATTR_ATIME_NOW) {
		new_inoattr->atime = curtime;
	}
	if (setmask & CFS_SET_ATTR_MTIME_NOW) {
		new_inoattr->mtime = curtime;
	}
	if (setmask & CFS_SET_ATTR_CTIME) {
		new_inoattr->ctime = inoattr->ctime;
	}
	uint64_t truncate_off = 0, truncate_len = 0;
	if (setmask & CFS_SET_ATTR_SIZE) {
		if ((setmask & CFS_SET_ATTR_TRUNCATE)) {
			if (new_inoattr->size > inoattr->size) {
				truncate_off = inoattr->size;
				truncate_len = new_inoattr->size - inoattr->size;
			} else {
				new_inoattr->size = inoattr->size;
			}
		} else {
			new_inoattr->size = new_inoattr->size > inoattr->size ? new_inoattr->size : inoattr->size;
		}
	}

	int ret = meta_.SetInodeAttr(ino, new_inoattr);

	if (ret != RET_OK) {
		//print some error;
		return ret;
	}

	if (truncate_len > 0) {
		//TODO: delete chunks;
	}
	return ret;
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
                    std::vector<struct InodeAttr*> *inodes, bool readdirplus) {

	//TODO: later here will add some meta_cache search

	return meta_.Readdir(pino, off, dentries, inodes, readdirplus);
}

int Cfs::CfsCreate(uint64_t pino, const char *name, uint32_t mode, struct InodeAttr *inoattr) {
	
	//TODO: put inoattr and parent_inoattr in meta cache

	struct InodeAttr parent_inoattr = {0};
	return meta_.Create(pino, name, mode, inoattr, &parent_inoattr);
	
}


int Cfs::CfsOpen(uint64_t ino, uint32_t flag, uint64_t *fh) {
	//check ino exist:
	struct InodeAttr inoattr = {0};
	int ret = meta_.GetInodeAttr(ino, &inoattr);
	if (ret == RET_NO_ENTRY) {
		return RET_ERR;
	}
	if (IS_INO_MARKDEL(inoattr.status)) {
		return RET_ERR;
	}

	//handle truncate:
	if ((flag & O_TRUNC) && ((flag & O_WRONLY) || (flag & O_RDWR))) {
		
	}

	//manage file handle
	struct CtxFile *ctxfile = NULL;
	std::map<uint64_t, struct CtxFile*>::iterator map_iter = ctx_files_.find(ino);
	if (map_iter != ctx_files_.end()) {
		ctxfile = map_iter->second;
		ctxfile->refcnt++;
	} else {
		ctxfile = (struct CtxFile*)malloc(sizeof(struct CtxFile));
		ctxfile->flag = flag;
		ctxfile->ino = ino;
		ctxfile->refcnt = 1;
		ctxfile->uid = inoattr.uid;
		ctxfile->gid = inoattr.gid;
		ctx_files_.emplace(ino, ctxfile);
		*fh = ino;
	}

	return 0;
	

}


int Cfs::CfsRead(uint64_t ino, uint64_t fh, int size, uint64_t off, char *buf) {
	if (size > CFS_MAX_BLK_SIZE || size <= 0) {
		//print some error;
		return -1;
	}

	uint64_t chunkid_1 = off / CFS_MAX_BLK_SIZE;
	uint64_t chunkoff_1 = off % CFS_MAX_BLK_SIZE;
	uint64_t chunkleft_1 = CFS_MAX_BLK_SIZE - chunkoff_1;
	int chunklen_1 = chunkleft_1 > size ? size : chunkleft_1;

	bool is_eof = false;
	int cnt = data_.Read(ino, chunkid_1, chunkoff_1, chunklen_1, buf, &is_eof);
	if (cnt == -1) {
		//print some error;
		return -1;
	}

	if (!is_eof && cnt < size) {
		buf += cnt;
		int chunklen_2 = size - cnt;
		uint64_t chunkid_2 = chunkid_1++;
		uint64_t chunkoff_2 = 0;
		int cnt_2 = data_.Read(ino, chunkid_2, chunkoff_2, chunklen_2, buf, &is_eof);
		if (cnt_2 == -1 && !is_eof) {
			//print some error .
			return -1;
		}
		cnt += cnt_2;
	}

	//update atime
	struct CtxFile *ctxfile = NULL;
	std::map<uint64_t, struct CtxFile*>::iterator map_iter = ctx_files_.find(ino);
	if (map_iter != ctx_files_.end()) {
		ctxfile = map_iter->second;
		if (!(ctxfile->flag | O_NOATIME)) {
			struct InodeAttr inoattr = {0};
			struct timespec curtime;
			clock_gettime(CLOCK_REALTIME, &curtime);
			inoattr.atime = curtime;
			int setmask = CFS_SET_ATTR_ATIME;
			struct InodeAttr new_inoattr = {0};
			CfsSetattr(ino, &inoattr, setmask, &new_inoattr);
		}
	}

	return cnt;
}


int Cfs::CfsWrite(uint64_t ino, uint64_t fh, int size, uint64_t off, char *buf) {
	if (size > CFS_MAX_BLK_SIZE || size <= 0) {
		//print some error;
		return -1;
	}

	uint64_t chunkid_1 = off / CFS_MAX_BLK_SIZE;
	uint64_t chunkoff_1 = off % CFS_MAX_BLK_SIZE;
	uint64_t chunkleft_1 = CFS_MAX_BLK_SIZE - chunkoff_1;
	int chunklen_1 = chunkleft_1 > size ? size : chunkleft_1;


	int cnt = data_.Write(ino, chunkid_1, chunkoff_1, chunklen_1, buf);
	if (cnt == -1 || cnt != chunklen_1) {
		//print some error;
		return -1;
	}

	if (cnt < size) {
		buf += cnt;
		int chunklen_2 = size - cnt;
		uint64_t chunkid_2 = chunkid_1++;
		uint64_t chunkoff_2 = 0;
		cnt = data_.Write(ino, chunkid_2, chunkoff_2, chunklen_2, buf);

		if (cnt != chunklen_2) {
			//print some error;
			return -1;
		}
	}


	//update inode attribute
	struct InodeAttr inoattr = {0};
	struct timespec curtime;
	clock_gettime(CLOCK_REALTIME, &curtime);
	inoattr.atime = curtime;
	inoattr.mtime = curtime;
	inoattr.ctime = curtime;
	inoattr.size = off + size;
	int setmask = CFS_SET_ATTR_ATIME | CFS_SET_ATTR_MTIME | CFS_SET_ATTR_CTIME | CFS_SET_ATTR_SIZE;
	struct InodeAttr new_inoattr = {0};
	CfsSetattr(ino, &inoattr, setmask, &new_inoattr);

	return size;
}


