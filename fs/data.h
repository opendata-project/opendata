#ifndef DATA_H_
#define DATA_H_


#include <stdint.h>
#include <sys/types.h>
#include <string>

class Data {
public:
    void Init();

public:
    int Read(uint64_t ino, uint64_t chunkid, uint64_t chunkoff, int chunklen, char *buf, bool *is_eof);
    int Write(uint64_t ino, uint64_t chunkid, uint64_t chunkoff, int chunklen, char *buf);

private:
    std::string chunk_path_ = "/opt/cfs_meta_data/datastore/";

};





#endif