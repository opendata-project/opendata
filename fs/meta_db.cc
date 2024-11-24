
#include "meta_db.h"

#include <iostream>
#include "cfs.h"


MetaDB::MetaDB() {

}

MetaDB::~MetaDB() {

}

void MetaDB::InitMetaDB() {

    std::string kDBPath = "/opt/tmp/fsmeta_rocksdb";

    // open DB
    Options options;
    TransactionDBOptions txn_db_options;
    options.create_if_missing = true;
    TransactionDB* db;
    Status s = TransactionDB::Open(options, txn_db_options, kDBPath, &db);
    assert(s.ok());

    // create table for inode
    ColumnFamilyHandle* table_inode;
    s = db->CreateColumnFamily(ColumnFamilyOptions(), "table_inode", &table_inode);
    assert(s.ok());

    //create table for dentry
    ColumnFamilyHandle* table_dentry;
    s = db->CreateColumnFamily(ColumnFamilyOptions(), "table_dentry", &table_dentry);
    assert(s.ok());

    //close DB
    s = db->DestroyColumnFamilyHandle(table_inode);
    assert(s.ok());
    s = db->DestroyColumnFamilyHandle(table_dentry);
    assert(s.ok());
    delete db;

    
    // open DB with column families
    std::vector<ColumnFamilyDescriptor> column_families;
    // open default column family
    column_families.push_back(ColumnFamilyDescriptor(
        rocksdb::kDefaultColumnFamilyName, ColumnFamilyOptions()));
    // open table_inode and table_dentry
    column_families.push_back(
        ColumnFamilyDescriptor("table_inode", ColumnFamilyOptions()));
    column_families.push_back(
        ColumnFamilyDescriptor("table_dentry", ColumnFamilyOptions()));

    s = TransactionDB::Open(options, txn_db_options, kDBPath, column_families, &table_handles_, &rocksdb_);
    assert(s.ok());

    /*
    // put and get from non-default column family
    s = db->Put(WriteOptions(), handles[1], Slice("key"), Slice("value"));
    assert(s.ok());
    std::string value;
    s = db->Get(ReadOptions(), handles[1], Slice("key"), &value);
    assert(s.ok());

    // atomic write
    WriteBatch batch;
    batch.Put(handles[0], Slice("key2"), Slice("value2"));
    batch.Put(handles[1], Slice("key3"), Slice("value3"));
    batch.Delete(handles[0], Slice("key"));
    s = db->Write(WriteOptions(), &batch);
    assert(s.ok());
    */
}

void MetaDB::CloseMetaDB() {

    Status s;
    /*
    // drop column family
    s = db->DropColumnFamily(handles[1]);
    assert(s.ok());
    */

    // close db
    for (auto handle : table_handles_) {
        s = rocksdb_->DestroyColumnFamilyHandle(handle);
        assert(s.ok());
    }
    delete rocksdb_;
}


int MetaDB::GetInodeAttr(uint64_t ino, struct InodeAttr *inoattr) {
    std::string tmp;
    Status s = rocksdb_->Get(read_options_, table_handles_[TABLE_INODE], Slice((char *)&ino, sizeof(ino)), &tmp);
    if (s.ok()) {
        memcpy(inoattr, tmp.data(), sizeof(struct InodeAttr));
        return RET_OK;
    } else {
        std::cout << s.ToString() << std::endl;
        return RET_ERR;
    }
}


int MetaDB::Lookup(uint64_t parent, const char *name, uint64_t *inode, struct InodeAttr *inoattr) {

    if (strlen(name) > FS_MAX_FILENAME_LEN) {
        return RET_ERR;
    }
    int keylen = 0;
    char keybuf[FS_MAX_FILENAME_LEN + sizeof(uint64_t)] = {0};
    EncodeDentryKey(parent, name, keybuf, &keylen);
    
    //query TABLE_DENTRY
    std::string tmp;
    Status s = rocksdb_->Get(read_options_, table_handles_[TABLE_DENTRY], Slice(keybuf, keylen), &tmp);
    if (s.ok()) {
        *inode = *(uint64_t *)tmp.data();
    } else {
        std::cout << s.ToString() << std::endl;
        return RET_ERR;
    }

    //query TABLE_INODE
    s = rocksdb_->Get(read_options_, table_handles_[TABLE_INODE], Slice((char*)inode, sizeof(*inode)), &tmp);
    if (s.ok()) {
        memcpy(inoattr, tmp.data(), sizeof(struct InodeAttr));
        return RET_OK;
    } else {
        std::cout << s.ToString() << std::endl;
        return RET_ERR;
    }

}


void MetaDB::EncodeDentryKey(uint64_t parent, const char *name, char *keybuf, int *keylen) {

    memcpy(keybuf, (char*)parent, sizeof(parent));
    *keylen += sizeof(parent);
    keybuf += sizeof(parent);

    int namelen = strlen(name);
    memcpy(keybuf, name, namelen);
    *keylen += namelen;
}
