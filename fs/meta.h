/*
 * define cluster file system meta functionalities
*/


#ifndef FS_META_H_
#define FS_META_H_

#include "meta_db.h"

class Meta {
private:

public:
    Meta();
    ~Meta();
    void Init();
    void Finish();

public:
    int GetInodeAttr(uint64_t ino, struct InodeAttr *inoattr);
    int Lookup(uint64_t parent, const char *name, uint64_t *inode, struct InodeAttr *inoattr);

private:


private:
    MetaDB metadb_;



};




#endif
