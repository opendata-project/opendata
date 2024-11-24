
/*
 * define cluster file system data functionalities
*/


#ifndef FS_CFS_H_
#define FS_CFS_H_

#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>


#define FS_MAX_FILENAME_LEN 256
#define FS_AVG_FILENAME_LEN 32

#include "meta.h"

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
};

struct DirEntry {
    uint64_t pino;
    uint64_t off;
    uint64_t ino;
	uint32_t namelen;
	char name[];
};


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

public:
    int CfsGetattr(uint64_t ino, struct InodeAttr *inoattr);
    void CfsRead(/*args*/);
    int CfsReaddir(uint64_t pino, int off, std::vector<struct DirEntry*> *dentries, 
                    std::vector<struct InodeAttr*> *inodes);
    int CfsLookup(uint64_t parent, const char *name, uint64_t *inode, struct InodeAttr *inoattr);
    void CfsOpen(/* args */);

public: 

    int CfsGetattrTest(uint64_t ino, struct stat *stbuf);

};


enum CFS_RETCODE {
    RET_OK = 0,
    RET_ERR = 1,
    RET_NO_ENTRY = 2, 
};





#endif