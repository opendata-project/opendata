
#include "meta.h"



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

int Meta::Lookup(uint64_t parent, const char *name, uint64_t *inode, struct InodeAttr *inoattr) {
    return metadb_.Lookup(parent, name, inode, inoattr);
}


int Meta::Readdir(uint64_t pino, int off, std::vector<struct DirEntry*> *dentries, 
                    std::vector<struct InodeAttr*> *inodes, bool readdirplus) {
    return metadb_.Readdir(pino, off, dentries, inodes, readdirplus);
}

int Meta::Create(uint64_t pino, const char *name, uint32_t mode, struct InodeAttr *inoattr, struct InodeAttr *parent_inoattr) {
    return metadb_.Create(pino, name, mode, inoattr, parent_inoattr);
}