
#include "meta.h"
#include "cfs.h"
#include "defines.h"
#include "../util/log.h"


Meta::Meta() {

}

Meta::~Meta() {

}

void Meta::Init() {
    metadb_.InitMetaDB();

}


void Meta::Finish() {
    metadb_.CloseMetaDB();
}

int Meta::GetInodeAttr(uint64_t ino, struct InodeAttr *inoattr) {
 
    return metadb_.GetInodeAttr(ino, inoattr);
}


int Meta::SetInodeAttr(uint64_t ino, struct InodeAttr *inoattr) {
    return metadb_.SetInodeAttr(ino, inoattr);
}

int Meta::Lookup(uint64_t parent, const char *name, uint64_t *inode, struct InodeAttr *inoattr) {
    return metadb_.Lookup(parent, name, inode, inoattr);
}


int Meta::Readdir(uint64_t pino, int off, std::vector<struct DirEntry*> *dentries, 
                    std::vector<struct InodeAttr*> *inodes, bool readdirplus) {
    int ret = metadb_.Readdir(pino, off, dentries, inodes, readdirplus);

	// for (int i = 0; i < dentries->size(); i++) {
    //     SPDLOG_INFO("dentry: pino={}, name={}, ino={}", (*dentries)[i]->pino, (*dentries)[i]->name, (*dentries)[i]->ino);
	// }
    // for (int i = 0; i < inodes->size(); i++) {
    //     SPDLOG_INFO("inode: ino={}, attr.mode={0:o}, attr.pino={}", (*inodes)[i]->ino, (*inodes)[i]->mode, (*inodes)[i]->pino);
    // }

    return ret;
}

int Meta::Create(uint64_t pino, const char *name, uint32_t mode, struct InodeAttr *inoattr, struct InodeAttr *parent_inoattr) {
    return metadb_.Create(pino, name, mode, inoattr, parent_inoattr);
}

int Meta::Unlink(uint64_t pino, const char *name) {
    return metadb_.Unlink(pino, name);
}

int Meta::Rmdir(uint64_t pino, const char *name, bool should_empty) {
    uint64_t rm_pino = 0;
	struct InodeAttr rm_pinoattr = {0};
	int ret = metadb_.Lookup(pino, name, &rm_pino, &rm_pinoattr);
	if (ret == RET_ENOENT) {
		return RET_OK;
	}

	if (!S_ISDIR(rm_pinoattr.mode)) {
		return RET_ENOTDIR;
	}
	
	std::vector<struct DirEntry*> dentries;
	std::vector<struct InodeAttr*> inodes;

	//TODO: now should_empty should be true.
	assert(should_empty);

	Readdir(rm_pino, 0, &dentries, &inodes, true);
	if (dentries.size() > 2) {      //. and ..
		return RET_ENOTEMPTY;
	}

    return metadb_.Rmdir(pino, name, should_empty);

}
