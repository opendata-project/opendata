/* 
 * meta process for file system daemon.
 * this daemon process can run on storage Node
*/

#include <iostream>
#include <fstream>
#include <time.h>
#include <vector>
#include <string.h>
#include <iostream>
#include <errno.h>
#include "rocksdb/db.h"
#include "rocksdb/options.h"
#include "rocksdb/slice.h"
#include "rocksdb/utilities/transaction_db.h"

#include "../util/log.h"

using rocksdb::DB;
using rocksdb::TransactionDB;
using rocksdb::Options;
using rocksdb::DBOptions;
using rocksdb::ReadOptions;
using rocksdb::WriteOptions;
using rocksdb::TransactionOptions;
using rocksdb::TransactionDBOptions;
using rocksdb::Status;
using rocksdb::ColumnFamilyHandle;
using rocksdb::Slice;
using rocksdb::ColumnFamilyOptions;
using rocksdb::ColumnFamilyDescriptor;
using rocksdb::Transaction;


struct InodeStat {
    uint64_t ino;
    uint32_t uid;
    uint32_t gid;
    uint32_t mode;
    uint32_t nlink;
    uint64_t size;
    uint64_t pino;
    uint32_t status;            //running status: mark delete, ...
    uint32_t padding;
};


static void GenerateChunkName(uint64_t ino, uint64_t chunkid, char*chunkname) {
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


int main(int argc, char *argv[]) {

	char tmp[100] = {0};
	int len = sprintf(tmp, "%llu", (long long unsigned int)(-1));

	system("rm -rf /opt/cfs_meta_data/fsmeta_rocksdb");
    system("rm -rf /opt/cfs_meta_data/datastore/");
	system("rm -rf /opt/cfs_meta_data/test*.log*");

	InitLogUtil();
	SPDLOG_DEBUG("HELLO, FUCK YOU");

	using namespace rocksdb;

	std::string kDBPath2 = "/opt/cfs_meta_data/fsmeta_rocksdb";
	std::string chunk_path_ = "/opt/cfs_meta_data/datastore/";


	char chunkname[32] = {0};
	int chunklen = 2;
	char *buf = (char*)malloc(100);
	buf[0] = '5';
	buf[1] = 0;
    GenerateChunkName(2, 1, chunkname);
    std::string chunkfullpath = chunk_path_ + std::string(chunkname);
	if (access(chunk_path_.c_str(), 0) != 0) {
		mkdir(chunk_path_.c_str(), 0777);
	}
    int fd = open(chunkfullpath.c_str(), O_CREAT | O_RDWR, 0777);
    if (fd == -1) {
		printf("%d\n", errno);
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



/*
	DB *db;
	Options options;
	options.IncreaseParallelism();
 
	options.create_if_missing = true;
 
	//open DB
	Status s=DB::Open(options,kDBPath2,&db);
	std::cout << s.ToString() << std::endl;
	assert(s.ok());
 
	struct InodeStat inoattr = {0};
	inoattr.gid = 1000;
	inoattr.uid = 1000;
	inoattr.ino = 2;
	inoattr.mode = 32404;
	inoattr.nlink = 1;
	inoattr.pino = 1;
	inoattr.size = 0;

	uint64_t ino = 2;
    s = db->Put(WriteOptions(), Slice((char*)&ino, sizeof(ino)), Slice((char*)&inoattr, sizeof(inoattr)));
    assert(s.ok());

    //for test begin:
    struct InodeStat inoattr_2 = {0};
	std::string tmp;
    s = db->Get(ReadOptions(), Slice((char*)&ino, sizeof(ino)), &tmp);
    if (s.ok()) {
		assert(tmp.size() == sizeof(struct InodeStat));
        memcpy(&inoattr_2, tmp.data(), sizeof(struct InodeStat));
    }

	//Put key-value
	s = db->Put(WriteOptions(),"key1","value1");
	assert(s.ok());

	uint64_t ino = 10;
	s = db->Put(WriteOptions(), Slice((char*)&ino, sizeof(ino)), "value2");
	assert(s.ok());
 
	std::string value;
	//get value
	s = db->Get(ReadOptions(),"key1",&value);
	assert(s.ok());
	std::cout << value << std::endl;

	std::string tmp1;
	s = db->Get(ReadOptions(), Slice((char*)&ino, sizeof(ino)), &tmp1);
	assert(s.ok());
	std::cout << ino << std::endl << tmp1 << std::endl;

	s = db->Put(WriteOptions(), "key21", "value21");
	assert(s.ok());

	s = db->Put(WriteOptions(), "key22", "value22");
	assert(s.ok());

	s = db->Put(WriteOptions(), "key23", "value23");
	assert(s.ok());

	s = db->Put(WriteOptions(), "key4", "value4");
	assert(s.ok());

	s = db->Get(ReadOptions(), Slice("key21", strlen("key21")), &tmp1);
	assert(s.ok());
	std::cout << "wwwkey21:" << tmp1 << std::endl;

	rocksdb::Iterator *iter = db->NewIterator(ReadOptions());
	iter->Seek(Slice("key5", sizeof("key5")));
	while (iter->Valid()) {
		std::cout << "enter here:" << std::endl;
		std::cout << iter->key().ToString() << "-:-" << iter->value().ToString() << std::endl;
		iter->Next();
	}
*/

	// int i = 20;
	// std::string x = "everybody";
	// SPDLOG_INFO("LOG LEVEL: {}", SPDLOG_ACTIVE_LEVEL);
	// SPDLOG_DEBUG("Hello, {}!", "World");
	// int x = 3;
	// const char * y ="edu.cn";
	// char *z = (char*)y;
	// SPDLOG_INFO("x={}, y={}", x, z);


    return 0;
}
