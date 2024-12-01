/* 
 * meta process for file system daemon.
 * this daemon process can run on storage Node
*/

#include <iostream>
#include <fstream>
#include <time.h>

#include <string.h>
#include <iostream>
#include "rocksdb/db.h"
#include "rocksdb/options.h"
#include "rocksdb/slice.h"

std::string kDBPath = "/opt/tmp/rocksdb_example";

int main(int argc, char *argv[]) {

    //std::ofstream fout("/opt/tmp/log.txt");
    //std::streambuf *sout = std::cout.rdbuf(fout.rdbuf());


    struct timespec ctime;
    std::cout << sizeof(ctime)  << std::endl;
    std::cout << sizeof(int) << std::endl;
    std::cout << sizeof(long int) << std::endl;

	using namespace rocksdb;
	DB *db;
	Options options;
	options.IncreaseParallelism();
 
	options.create_if_missing = true;
 
	//open DB
	Status s=DB::Open(options,kDBPath,&db);
	assert(s.ok());
 
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


    //fout.close();
    return 0;
}
