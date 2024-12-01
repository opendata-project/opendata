
/*
 * define cluster file system data functionalities
*/


#ifndef FS_CFS_H_
#define FS_CFS_H_

#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "meta.h"

#define FS_MAX_FILENAME_LEN 255     //max file name string len : 255
#define FS_AVG_FILENAME_LEN 32

#define CFS_EOF -1

#define IS_INO_MARKDEL(status)     ((status) & 0x00000001)

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


struct CtxFile {
    uint64_t ino;
    uint32_t flag;
    uint32_t mode;
    uint32_t refcnt;
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
    std::map<uint64_t, struct CtxFile*> ctx_files_;
    typedef std::map<uint64_t, struct CtxFile*> MapCtxFiles_t;

public:
    int CfsGetattr(uint64_t ino, struct InodeAttr *inoattr);
    void CfsRead(/*args*/);
    int CfsReaddir(uint64_t pino, int off, std::vector<struct DirEntry*> *dentries, 
                    std::vector<struct InodeAttr*> *inodes, bool readdirplus);
    int CfsLookup(uint64_t parent, const char *name, uint64_t *inode, struct InodeAttr *inoattr);
    int CfsOpen(uint64_t ino, uint32_t flag, uint64_t *fh);
    int CfsCreate(uint64_t pino, const char *name, uint32_t mode, struct InodeAttr *inoattr);

public: 

    int CfsGetattrTest(uint64_t ino, struct stat *stbuf);

};


enum CFS_RETCODE {
    RET_OK = 0,
    RET_ERR = 1,
    RET_NO_ENTRY = 2, 
    RET_EXIST = 3,
};





#endif