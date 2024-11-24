
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
