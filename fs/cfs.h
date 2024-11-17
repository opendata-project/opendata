
/*
 * define cluster file system data functionalities
*/


#ifndef CFS_H_
#define CFS_H_

#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>




class Cfs
{
private:
    /* data */
public:
    Cfs(/* args */);
    ~Cfs();

    int CfsGetattr(uint64_t ino, struct stat *stbuf);
    void CfsRead(/*args*/);
    void CfsReaddir(/*args*/);
    int CfsLookup(uint64_t parent, const char *name, uint64_t *inode, struct stat *stbuf);
    void CfsOpen(/* args */);

};






#endif