
#ifndef DEFINES_H_
#define DEFINES_H_



#define CFS_EOF -1
#define CFS_MAX_FILENAME_LEN    255             //max file name string len : 255
#define CFS_AVG_FILENAME_LEN    32
#define CFS_MAX_BLK_SIZE        4*1024*1024     //max block size: read, write, chunk

#define IS_INO_MARKDEL(status)     ((status) & 0x00000001)


/*set mask of  setattr*/
#define CFS_SET_ATTR_MODE       (1 << 0)
#define CFS_SET_ATTR_UID        (1 << 1)
#define CFS_SET_ATTR_GID        (1 << 2)
#define CFS_SET_ATTR_SIZE       (1 << 3)
#define CFS_SET_ATTR_ATIME      (1 << 4)
#define CFS_SET_ATTR_MTIME      (1 << 5)
#define CFS_SET_ATTR_ATIME_NOW  (1 << 7)
#define CFS_SET_ATTR_MTIME_NOW  (1 << 8)
#define CFS_SET_ATTR_FORCE      (1 << 9)
#define CFS_SET_ATTR_CTIME      (1 << 10)
#define CFS_SET_ATTR_KILL_SUID  (1 << 11)
#define CFS_SET_ATTR_KILL_SGID  (1 << 12)
#define CFS_SET_ATTR_TRUNCATE   (1 << 13)


enum CFS_RETCODE {
    RET_ERR         = -1,
    RET_OK          = 0,
    RET_NO_ENTRY    = 1, 
    RET_EXIST       = 2,
};


#endif