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
	s = db->Put(WriteOptions(),"key1","value");
	assert(s.ok());

	uint64_t ino = 10;
	s = db->Put(WriteOptions(), Slice((char*)&ino, sizeof(ino)), "value");
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

	struct _st {
		char b[20];
	} _internal;
	memset(&_internal, 0, sizeof(_internal));
    std::string tmp;
    s = db->Get(ReadOptions(), Slice("key1"), &tmp);
    if (s.ok()) {
        memcpy(&_internal, tmp.data(), sizeof(char) * tmp.size());
    } else {
        std::cout << s.ToString() << std::endl;
    }
    std::cout << (char*)&_internal << std::endl;

    //fout.close();
    return 0;
}
