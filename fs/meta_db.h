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

#define SEQ_NAME_NAXLEN       32
#define SEQ_UPPER_STRIDE      10000
#define SEQ_LOWWER_STRIDE     2000
#define SEQ_ID_BEGIN_         2

#define MAX_DENTRY_KEY_LEN      FS_MAX_FILENAME_LEN + sizeof(uint64_t)


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


class MetaDB;
class SeqAllocator {
public:
    SeqAllocator() {
        seq_id_.store(SEQ_ID_BEGIN_);
    }

public:
    void SeqInit(const char* seqname, MetaDB *ptr);
    uint64_t SeqGetone();

private:
    char seq_name_[SEQ_NAME_NAXLEN];
    uint64_t seq_id_persist_;
    std::atomic<uint64_t> seq_id_;
    MetaDB *dbptr_;
};




//actually here should be a base MetaDB class,
//implementation of different KVDB should be inherit from base class
//leave to finish

class MetaDB {

public:
    enum {
        TABLE_DEFAULT = 0,          //KEY-VALUE: misc: (1). max inode number; (2). 
        TABLE_INODE = 1,            //KEY-VALUE: inode <-> inode attribute;
        TABLE_DENTRY=2,             //KEY-VALUE: parent inode + name <-> inode;
    };
    MetaDB();
    ~MetaDB();
    void InitMetaDB();
    void CloseMetaDB();

public:
    int PersistKV(const char*key, int klen, const char *value, int vlen);       //for some special persist info
    int GetKV(const char *key, int klen, char *value, int *vlen);

public:
    //TODO: actually here should not appear file semantic
    //TODO: leave for future to decouple it.
    int GetInodeAttr(uint64_t ino, struct InodeAttr *inoattr);
    int Lookup(uint64_t parent, const char *name, uint64_t *inode, struct InodeAttr *inoattr);
    int Readdir(uint64_t pino, int off, std::vector<struct DirEntry*> *dentries, 
                    std::vector<struct InodeAttr*> *inodes, bool readdirplus);
    int Create(uint64_t pino, const char *name, uint32_t mode, struct InodeAttr *inoattr, struct InodeAttr *parent_inoattr);

private:
    void InitRootDirInode();
    int FillDefaultDentry(uint64_t pino, int off, std::vector<struct DirEntry*> *dentries, 
                            std::vector<struct InodeAttr*> *inodes, bool readdirplus); 

    //encode  dentry key and value
    void EncodeDentryKey(uint64_t parent, const char *name, char *keybuf, int *keylen);
    void DecodeDentryKey(char *keybuf, int keylen, uint64_t *pino, char *name);

private:
    Options options_;
    ReadOptions read_options_;
    WriteOptions write_options_;
    TransactionOptions txn_options_;
    TransactionDBOptions txn_db_options_;

    TransactionDB *rocksdb_;
    std::vector<ColumnFamilyHandle*> table_handles_;

    SeqAllocator inode_allocator_;

};




#endif