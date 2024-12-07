
#include "data.h"
#include <unistd.h>
#include <string>
#include <fcntl.h>
#include "../util/log.h"

#define CFS_CHUNK_NAME_LEN	64


void Data::Init() {
    if (access(chunk_path_.c_str(), 0) != 0) {
		mkdir(chunk_path_.c_str(), 0777);
	}
}

void GenerateChunkName(uint64_t ino, uint64_t chunkid, char*chunkname) {
	chunkname[0] = 'i';
	chunkname += 1;
    int cnt = sprintf(chunkname, "%llu", (long long unsigned int)ino);
    chunkname += cnt;
	chunkname[0] = '_';
	chunkname += 1;
	chunkname[0] = 'c';
	chunkname += 1; 
    cnt = sprintf(chunkname, "%llu", (long long unsigned int)chunkid);
	chunkname += cnt;
	chunkname[0] = 0;
}

int Data::Read(uint64_t ino, uint64_t chunkid, uint64_t chunkoff, int chunklen, char *buf, bool *is_eof) {

    char chunkname[CFS_CHUNK_NAME_LEN] = {0};
    GenerateChunkName(ino, chunkid, chunkname);
    std::string chunkfullpath = chunk_path_ + std::string(chunkname);
    int fd = open(chunkfullpath.c_str(), O_RDONLY);
    if (fd == -1) {
        //print some error.
        return -1;
    }
    
    *is_eof = false;
    int size = 0, cnt = 0;
    while (size < chunklen) {
        cnt = read(fd, buf, chunklen - size);
        if (cnt > 0) {
            size += cnt;
            buf += cnt;
        } else if (cnt == 0) {
            *is_eof = true;
            break;
        } else {
            //print some error.
        }
    }
    close(fd);
    return size;
}


int Data::Write(uint64_t ino, uint64_t chunkid, uint64_t chunkoff, int chunklen, char *buf) {
    char chunkname[CFS_CHUNK_NAME_LEN] = {0};
    GenerateChunkName(ino, chunkid, chunkname);
    std::string chunkfullpath = chunk_path_ + std::string(chunkname);
    int fd = open(chunkfullpath.c_str(), O_CREAT | O_RDWR, 0777);
    if (fd == -1) {
        //print some error.
        return -1;
    }
    int size = 0, cnt = 0;
    while (size < chunklen) {
        cnt = write(fd, buf, chunklen - size);
        if (cnt > 0) {
            size += cnt;
            buf += cnt;
        }
    }
    close(fd);
    return size;
}
