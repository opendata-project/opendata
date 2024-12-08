/*
 * define cluster file system meta functionalities
*/


#ifndef FS_META_H_
#define FS_META_H_

#include "meta_db.h"

class Data;

class Meta {
private:

public:
    Meta();
    ~Meta();
    void Init(Data *data);
    void Finish();

public:
    int GetInodeAttr(uint64_t ino, struct InodeAttr *inoattr);
    int SetInodeAttr(uint64_t ino, struct InodeAttr *inoattr);
    int Lookup(uint64_t parent, const char *name, uint64_t *inode, struct InodeAttr *inoattr);
    int Readdir(uint64_t pino, int off, std::vector<struct DirEntry*> *dentries, 
                    std::vector<struct InodeAttr*> *inodes, bool readdirplus);
    int Create(uint64_t pino, const char *name, uint32_t mode, struct InodeAttr *inoattr, struct InodeAttr *parent_inoattr);
    int Unlink(uint64_t pino, const char *name);
    int Rmdir(uint64_t pino, const char *name, bool should_emtpy);

    int Rename(uint64_t pino, const char *name, uint64_t newpino, const char *newname);

public:
    void RunBgTask();
    void DeleteFile(uint64_t ino, uint64_t length);

private:
    MetaDB metadb_;
    Data *pdata_;

};




#endif
