/*
* database for storing cluster file system metadata
*/

#ifndef FS_META_DB_H_
#define FS_META_DB_H_

#include <vector>

#include "rocksdb/db.h"
#include "rocksdb/options.h"
#include "rocksdb/slice.h"
#include "rocksdb/utilities/transaction_db.h"


using rocksdb::DB;
using rocksdb::TransactionDB;
using rocksdb::Options;
using rocksdb::DBOptions;
using rocksdb::ReadOptions;
using rocksdb::WriteOptions;
using rocksdb::TransactionDBOptions;
using rocksdb::Status;
using rocksdb::ColumnFamilyHandle;
using rocksdb::Slice;
using rocksdb::ColumnFamilyOptions;
using rocksdb::ColumnFamilyDescriptor;

//actually here should be a base MetaDB class,
//implementation of different KVDB should be inherit from base class
//leave to finish

class MetaDB {

public:
    enum {
        TABLE_INODE = 1,            //KEY-VALUE: inode <-> inode attribute;
        TABLE_DENTRY=2,             //KEY-VALUE: parent inode + name <-> inode;
    };
    MetaDB();
    ~MetaDB();
    void InitMetaDB();
    void CloseMetaDB();

public:
    //TODO: actually here should not appear file semantic
    //TODO: leave for future to decouple it.
    int GetInodeAttr(uint64_t ino, struct InodeAttr *inoattr);
    int Lookup(uint64_t parent, const char *name, uint64_t *inode, struct InodeAttr *inoattr);

private:
    //encode  dentry key and value
    void EncodeDentryKey(uint64_t parent, const char *name, char *keybuf, int *keylen);

private:
    Options options_;
    ReadOptions read_options_;
    WriteOptions write_options_;
    TransactionDBOptions txn_db_options;

    TransactionDB *rocksdb_;
    std::vector<ColumnFamilyHandle*> table_handles_;
};




#endif