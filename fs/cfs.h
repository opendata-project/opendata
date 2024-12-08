
/*
 * define cluster file system data functionalities
*/


#ifndef CFS_H_
#define CFS_H_

#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "meta.h"
#include "data.h"



struct InodeAttr {
    uint64_t ino;
    uint32_t uid;
    uint32_t gid;
    struct timespec ctime;
    struct timespec mtime;
    struct timespec atime;
    uint32_t mode;
    uint32_t nlink;
    uint64_t size;
    uint64_t pino;
    uint32_t status;            //running status: mark delete, ...
    uint32_t padding;
};

struct DirEntry {
    uint64_t pino;
    uint64_t off;
    uint64_t ino;
	uint32_t namelen;
	char name[];
};

struct FileHandle {
    union {
        uint64_t file_handle;
        char file_cookie[56];
    };
};

// struct CtxFile {
//     uint64_t ino;
//     uint32_t flag;
//     uint32_t mode;
//     uint32_t refcnt;
//     uint32_t uid;
//     uint32_t gid;
// };



class Cfs {
public:
    Cfs();
    ~Cfs();
    void Init();
    void Finish();

private:
    /* fist iteration with single process for simplicity, 
    later will have dedicated metadata server */
    Meta meta_;

    Data data_;

    // std::map<uint64_t, struct CtxFile*> ctx_files_;         //TODO: key should be change to struct FileHandle;
    // typedef std::map<uint64_t, struct CtxFile*> MapCtxFiles_t;


public:
    int CfsGetattr(uint64_t ino, struct InodeAttr *inoattr);
    int CfsSetattr(uint64_t ino, struct InodeAttr *inoattr, int setmask, struct InodeAttr *new_inoattr);
    int CfsRead(uint64_t ino, uint64_t fh, int size, uint64_t off, char *buf);
    int CfsWrite(uint64_t ino, uint64_t fh, int size, uint64_t off, char *buf);
    int CfsReaddir(uint64_t pino, int off, std::vector<struct DirEntry*> *dentries, 
                    std::vector<struct InodeAttr*> *inodes, bool readdirplus);
    int CfsLookup(uint64_t parent, const char *name, uint64_t *inode, struct InodeAttr *inoattr);
    int CfsCreate(uint64_t pino, const char *name, uint32_t mode, struct InodeAttr *inoattr);

    int CfsOpen(uint64_t ino, uint32_t flag, uint64_t *fh);
    int CfsRelease(uint64_t ino, uint64_t fh);
    int CfsUnlink(uint64_t pino, const char *name);
    int CfsRmdir(uint64_t pino, const char *name, bool should_empty);

    int CfsRename(uint64_t pino, const char *name, uint64_t newpino, const char *newname);

public: 
    void CfsRunBgTask();
};





#endif