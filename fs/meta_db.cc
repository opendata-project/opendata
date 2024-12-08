
#include "meta_db.h"

#include <iostream>
#include "cfs.h"
#include "defines.h"
#include "../util/log.h"
#include "meta.h"


MetaDB::MetaDB() {

}

MetaDB::~MetaDB() {

}

void MetaDB::InitMetaDB(Meta *pmeta) {
    pmeta_ = pmeta;

    std::string kDBPath = "/opt/cfs_meta_data/fsmeta_rocksdb";

    // open DB
    Options options;
    TransactionDBOptions txn_db_options;
    options.create_if_missing = true;
    TransactionDB* db;
    Status s = TransactionDB::Open(options, txn_db_options, kDBPath, &db);
    SPDLOG_INFO("create db failed. error: {}", s.ToString());

    assert(s.ok());

    // create table for inode
    ColumnFamilyHandle* table_inode;
    s = db->CreateColumnFamily(ColumnFamilyOptions(), "table_inode", &table_inode);
    assert(s.ok());

    //create table for dentry
    ColumnFamilyHandle* table_dentry;
    s = db->CreateColumnFamily(ColumnFamilyOptions(), "table_dentry", &table_dentry);
    assert(s.ok());

    //create table for transh
    ColumnFamilyHandle* table_trash;
    s = db->CreateColumnFamily(ColumnFamilyOptions(), "table_trash", &table_trash);
    assert(s.ok());

    //close DB
    s = db->DestroyColumnFamilyHandle(table_inode);
    assert(s.ok());
    s = db->DestroyColumnFamilyHandle(table_dentry);
    assert(s.ok());
    s = db->DestroyColumnFamilyHandle(table_trash);
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
    column_families.push_back(
        ColumnFamilyDescriptor("table_trash", ColumnFamilyOptions()));

    s = TransactionDB::Open(options, txn_db_options, kDBPath, column_families, &table_handles_, &rocksdb_);
    assert(s.ok());


    InitRootDirInode();

}


//init root dir
void MetaDB::InitRootDirInode() {

    //init inode;
    struct InodeAttr inoattr = {0};
    inoattr.ino = 1;
    inoattr.pino = 1;
    inoattr.nlink = 2;
    inoattr.size = 4096;
    inoattr.uid = 0;
    inoattr.gid = 0;
    inoattr.mode = 0777 | S_IFDIR;
    struct timespec curtime;
    clock_gettime(CLOCK_REALTIME, &curtime);
    inoattr.atime = curtime;
    inoattr.ctime = curtime;
    inoattr.mtime = curtime;

    //insert inode
    Status s = rocksdb_->Put(write_options_, table_handles_[TABLE_INODE], 
                                Slice((char*)&inoattr.ino, sizeof(inoattr.ino)), 
                                Slice((char*)&inoattr, sizeof(inoattr)));
    assert(s.ok());
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

void MetaDB::CompareAndUpdateAttr(struct InodeAttr *newattr, struct InodeAttr *oldattr) {
    newattr->size = newattr->size > oldattr->size ? newattr->size : oldattr->size;
}

int MetaDB::SetInodeAttr(uint64_t ino, struct InodeAttr *inoattr) {

    //modify  attribute
    Transaction* txn = rocksdb_->BeginTransaction(write_options_, txn_options_);

    //get for update ino;
    std::string tmp; 
    Status s = txn->GetForUpdate(read_options_, table_handles_[TABLE_INODE], Slice((char*)&ino, sizeof(ino)), &tmp);
    if (!s.ok()) {
        txn->Rollback();
        return RET_ERR;
    }
    struct InodeAttr oldattr = {0};
    memcpy(&oldattr, tmp.data(), tmp.size());

    CompareAndUpdateAttr(inoattr, &oldattr);

    s = txn->Put(table_handles_[TABLE_INODE], Slice((char*)&ino, sizeof(ino)), Slice((char*)inoattr, sizeof(*inoattr)));
    if (!s.ok()) {
        txn->Rollback();
        return RET_ERR;
    }
    s = txn->Commit();
    if (!s.ok()) {
        txn->Rollback();
        return RET_ERR;
    }
    delete txn;
    
    return RET_OK;
}


int MetaDB::Lookup(uint64_t parent, const char *name, uint64_t *inode, struct InodeAttr *inoattr) {

    if (strlen(name) > CFS_MAX_FILENAME_LEN) {
        return RET_ERR;
    }
    int keylen = 0;
    char keybuf[MAX_DENTRY_KEY_LEN] = {0};
    EncodeDentryKey(parent, name, keybuf, &keylen);
    
    //query TABLE_DENTRY
    std::string tmp;
    Status s = rocksdb_->Get(read_options_, table_handles_[TABLE_DENTRY], Slice(keybuf, keylen), &tmp);
    if (s.ok()) {
        *inode = *(uint64_t *)tmp.data();
    } else if (s.IsNotFound()) {
        return RET_ENOENT;
    } else {
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

//name is a string, not binary
void MetaDB::EncodeDentryKey(uint64_t pino, const char *name, char *keybuf, int *keylen) {

    memcpy(keybuf, (char*)&pino, sizeof(pino));
    *keylen += sizeof(pino);
    keybuf += sizeof(pino);
    if (name != NULL) {
        int namelen = strlen(name);
        memcpy(keybuf, name, namelen);
        *keylen += namelen;
    }
}

//name is a string, not binary
void MetaDB::DecodeDentryKey(char *keybuf, int keylen, uint64_t *pino, char *name) {
    *pino = *(uint64_t*)keybuf;
    keylen -= sizeof(pino);
    keybuf += sizeof(pino);
    if (keylen > 0) {
        memcpy(name, keybuf, keylen);
        name[keylen] = 0;         //end of string
    }
}



int MetaDB::FillDefaultDentry(uint64_t pino, int off, 
                            std::vector<struct DirEntry*> *dentries, 
                            std::vector<struct InodeAttr*> *inodes, 
                            bool readdirplus) {
    
    //fill "." 
    struct DirEntry *dentry_dot = (struct DirEntry*)malloc(sizeof(struct DirEntry));
    dentry_dot->ino = pino;     //ino of current dir
    strcpy(dentry_dot->name, ".");
    dentry_dot->namelen = strlen(".");
    dentry_dot->off = CFS_EOF;
    dentries->push_back(dentry_dot);

    //get attribute of ".", never mind if readdir plus
    std::string tmp_dot;
    struct InodeAttr *inoattr_dot = (struct InodeAttr*)malloc(sizeof(struct InodeAttr));
    Status s = rocksdb_->Get(read_options_, table_handles_[TABLE_INODE], 
                            Slice((char *)&dentry_dot->ino, sizeof(dentry_dot->ino)), &tmp_dot);
    if (s.ok()) {
        memcpy(inoattr_dot, tmp_dot.data(), sizeof(struct InodeAttr));
    } else {
        //print some error here;
        memset(inoattr_dot, 0, sizeof(struct InodeAttr));
    }
    uint64_t ino_dotdot = inoattr_dot->pino;
    if (readdirplus) {
        inodes->push_back(inoattr_dot);
    } else {
        delete(inoattr_dot);
    }

    
    //fill “.."
    struct DirEntry *dentry_dotdot = (struct DirEntry*)malloc(sizeof(struct DirEntry));
    dentry_dotdot->ino = ino_dotdot;
    strcpy(dentry_dotdot->name, "..");
    dentry_dotdot->namelen = strlen("..");
    dentry_dotdot->off = CFS_EOF;
    dentries->push_back(dentry_dotdot);

    //if readdir plus, get attribute of ".."
    std::string tmp_dotdot;
    struct InodeAttr *inoattr_dotdot = (struct InodeAttr*)malloc(sizeof(struct InodeAttr));
    if (readdirplus) {
        Status s = rocksdb_->Get(read_options_, table_handles_[TABLE_INODE], 
                                Slice((char*)&dentry_dotdot->ino, sizeof(dentry_dotdot->ino)), &tmp_dotdot);
        if (s.ok()) {
            memcpy(inoattr_dotdot, tmp_dotdot.data(), sizeof(struct InodeAttr));
        } else {
            //print some error here;
            memset(inoattr_dotdot, 0, sizeof(struct InodeAttr));
        }
        inodes->push_back(inoattr_dotdot);
    } else {
        delete(inoattr_dotdot);
    }

    return 0;
}



int MetaDB::Readdir(uint64_t pino, int off, std::vector<struct DirEntry*> *dentries, 
                    std::vector<struct InodeAttr*> *inodes, bool readdirplus) {
    int keylen = 0;
    char keybuf[MAX_DENTRY_KEY_LEN] = {0};

    //fill default "." and ".."
    FillDefaultDentry(pino, off, dentries, inodes, readdirplus);

    //fill entries: start with prefix:pino
    EncodeDentryKey(pino, NULL, keybuf, &keylen);
    rocksdb::Iterator *iter = rocksdb_->NewIterator(read_options_, table_handles_[TABLE_DENTRY]);
    iter->Seek(Slice(keybuf, keylen));
    while (iter->Valid()) {
        char * tmpbuf = (char*)(iter->key().data());
        int tmplen = iter->key().size();
        int tmpnamelen = iter->key().size() - sizeof(uint64_t);
        if (*(uint64_t*)tmpbuf != pino) {
            break;
        }
        struct DirEntry *dentry = (struct DirEntry*)malloc(sizeof(struct DirEntry) + tmpnamelen + 1);
        DecodeDentryKey(tmpbuf, tmplen, &dentry->pino, dentry->name);
        dentry->namelen = tmpnamelen;
        dentry->off = CFS_EOF;
        dentry->ino = *(uint64_t*)(iter->value().data());
        dentries->push_back(dentry);
        iter->Next();
    }


    //fill inodes
    if (readdirplus) {
        std::string tmp;
        for (int i = 2; i < dentries->size(); i++) {    //begin with 2, because:(0:".", 1:"..")
            uint64_t ino = (*dentries)[i]->ino;
            struct InodeAttr *inoattr = (struct InodeAttr*)malloc(sizeof(struct InodeAttr));
            Status s = rocksdb_->Get(read_options_, table_handles_[TABLE_INODE], 
                                    Slice((char*)&ino, sizeof(ino)), &tmp);
            if (s.ok()) {
                memcpy(inoattr, tmp.data(), sizeof(struct InodeAttr));
            } else {
                //print some error;
                memset(inoattr, 0, sizeof(struct InodeAttr));
            }
            inodes->push_back(inoattr);
        }
    }

    //modify pino attribute: atime
    Transaction* txn = rocksdb_->BeginTransaction(write_options_, txn_options_);

    //get for update pino;
    std::string tmp; 
    Status s = txn->GetForUpdate(read_options_, table_handles_[TABLE_INODE], Slice((char*)&pino, sizeof(pino)), &tmp);
    if (!s.ok()) {
        txn->Rollback();
        return RET_ERR;
    }
    struct InodeAttr inoattr = {0};
    memcpy(&inoattr, tmp.data(), tmp.size());
    struct timespec curtime;
    clock_gettime(CLOCK_REALTIME, &curtime);
    inoattr.atime = curtime;
    s = txn->Put(table_handles_[TABLE_INODE], Slice((char*)&pino, sizeof(pino)), Slice((char*)&inoattr, sizeof(inoattr)));
    if (!s.ok()) {
        txn->Rollback();
        return RET_ERR;
    }
    s = txn->Commit();
    if (!s.ok()) {
        txn->Rollback();
        return RET_ERR;
    }
    delete txn;

    return RET_OK;
}


int MetaDB::Create(uint64_t pino, const char *name, uint32_t mode, struct InodeAttr *inoattr, struct InodeAttr *parent_inoattr) {

    Transaction* txn = rocksdb_->BeginTransaction(write_options_, txn_options_);

    int keylen = 0;
    char keybuf[MAX_DENTRY_KEY_LEN] = {0};
    EncodeDentryKey(pino, name, keybuf, &keylen);

    //get for update dentry: pino+name
    std::string tmp; 
    Status s = txn->GetForUpdate(read_options_, table_handles_[TABLE_DENTRY], Slice(keybuf, keylen), &tmp);
    if (!s.IsNotFound()) {
        txn->Rollback();
        return RET_EEXIST;
    }
    //get for update inode: pino
    s = txn->GetForUpdate(read_options_, table_handles_[TABLE_INODE], Slice((char*)&pino, sizeof(pino)), &tmp);
    if (!s.ok()) {
        txn->Rollback();
        return RET_ERR;
    }
    memcpy(parent_inoattr, tmp.data(), tmp.size());

    //insert dentry: pino+name <-> ino
    uint64_t ino = inode_allocator_.SeqGetone();
    s = txn->Put(table_handles_[TABLE_DENTRY], Slice(keybuf, keylen), Slice((char*)&ino, sizeof(ino)));
    if (!s.ok()) {
        txn->Rollback();
        return RET_ERR;
    }

    SPDLOG_INFO("insert dentry: pino={},name={},ino={}", pino, name, ino);

    //insert inode: ino
    inoattr->ino = ino;
    s = txn->Put(table_handles_[TABLE_INODE], Slice((char*)&ino, sizeof(ino)), Slice((char*)inoattr, sizeof(*inoattr)));
    if (!s.ok()) {
        txn->Rollback();
        return RET_ERR;
    }

    SPDLOG_INFO("insert inode: ino={}, attr.mode={0:o} attr.pino={}", ino, inoattr->mode, inoattr->pino);


    //update parent inode: pino
    parent_inoattr->ctime = inoattr->ctime;
    parent_inoattr->mtime = inoattr->mtime;
    if (S_ISDIR(mode)) {
        parent_inoattr->nlink++;
    }
    s = txn->Put(table_handles_[TABLE_INODE], Slice((char*)&pino, sizeof(pino)), Slice((char*)parent_inoattr, sizeof(*parent_inoattr)));
    if (!s.ok()) {
        txn->Rollback();
        return RET_ERR;
    }


    s = txn->Commit();
    if (!s.ok()) {
        txn->Rollback();
        return RET_ERR;
    }
    delete txn;


    return RET_OK;
}


int MetaDB::Unlink(uint64_t pino, const char *name) {

    Transaction* txn = rocksdb_->BeginTransaction(write_options_, txn_options_);

    int keylen = 0;
    char keybuf[MAX_DENTRY_KEY_LEN] = {0};
    EncodeDentryKey(pino, name, keybuf, &keylen);

    //get for update dentry: pino+name
    std::string tmp; 
    Status s = txn->GetForUpdate(read_options_, table_handles_[TABLE_DENTRY], Slice(keybuf, keylen), &tmp);
    if (s.IsNotFound()) {
        txn->Rollback();
        return RET_OK;
    }
    uint64_t ino = *(uint64_t*)tmp.data();

    //get for update inode: pino
    struct InodeAttr parent_inoattr = {0};
    s = txn->GetForUpdate(read_options_, table_handles_[TABLE_INODE], Slice((char*)&pino, sizeof(pino)), &tmp);
    if (!s.ok()) {
        txn->Rollback();
        return RET_ERR;
    }
    memcpy(&parent_inoattr, tmp.data(), tmp.size());

    //get for update inode: ino
    struct InodeAttr inoattr = {0};
    s = txn->GetForUpdate(read_options_, table_handles_[TABLE_INODE], Slice((char*)&ino, sizeof(ino)), &tmp);
    if (!s.ok()) {
        txn->Rollback();
        return RET_ERR;
    }
    memcpy(&inoattr, tmp.data(), tmp.size());

    //TODO: wrapper in funciton process later
    struct timespec curtime;
    clock_gettime(CLOCK_REALTIME, &curtime);

    inoattr.nlink--;
    inoattr.ctime = curtime;
    inoattr.mtime = curtime;

    if (inoattr.nlink == 0) {
        //delete dentry
        txn->Delete(table_handles_[TABLE_DENTRY], Slice(keybuf, keylen));
        //delete inode
        txn->Delete(table_handles_[TABLE_INODE], Slice((char*)&ino, sizeof(ino)));
        //add inode to trash
        txn->Put(table_handles_[TABLE_TRASH], Slice((char*)&ino, sizeof(ino)), Slice((char*)&inoattr, sizeof(inoattr)));
    } else {
        //only delete this dentry
        txn->Delete(table_handles_[TABLE_DENTRY], Slice(keybuf, keylen));
        //update inode
        txn->Put(table_handles_[TABLE_INODE], Slice((char*)&ino, sizeof(ino)), Slice((char*)&inoattr, sizeof(inoattr)));
    }

    //update parent inode: pino
    parent_inoattr.ctime = curtime;
    parent_inoattr.mtime = curtime;
    s = txn->Put(table_handles_[TABLE_INODE], Slice((char*)&pino, sizeof(pino)), Slice((char*)&parent_inoattr, sizeof(parent_inoattr)));

    s = txn->Commit();
    if (!s.ok()) {
        txn->Rollback();
        return RET_ERR;
    }
    delete txn;

    return RET_OK;

}


int MetaDB::Rmdir(uint64_t pino, const char *name, bool should_empty) {

    Transaction* txn = rocksdb_->BeginTransaction(write_options_, txn_options_);

    int keylen = 0;
    char keybuf[MAX_DENTRY_KEY_LEN] = {0};
    EncodeDentryKey(pino, name, keybuf, &keylen);

    //get for update dentry: pino+name
    std::string tmp; 
    Status s = txn->GetForUpdate(read_options_, table_handles_[TABLE_DENTRY], Slice(keybuf, keylen), &tmp);
    if (s.IsNotFound()) {
        txn->Rollback();
        return RET_OK;
    }
    uint64_t ino = *(uint64_t*)tmp.data();


    //get for update inode: ino
    struct InodeAttr inoattr = {0};
    s = txn->GetForUpdate(read_options_, table_handles_[TABLE_INODE], Slice((char*)&ino, sizeof(ino)), &tmp);
    if (!s.ok()) {
        txn->Rollback();
        return RET_ERR;
    }
    memcpy(&inoattr, tmp.data(), tmp.size());    


    //get for update inode: pino
    struct InodeAttr parent_inoattr = {0};
    s = txn->GetForUpdate(read_options_, table_handles_[TABLE_INODE], Slice((char*)&pino, sizeof(pino)), &tmp);
    if (!s.ok()) {
        txn->Rollback();
        return RET_ERR;
    }
    memcpy(&parent_inoattr, tmp.data(), tmp.size());


    //TODO: wrapper in funciton process later

    //delete dentry
    txn->Delete(table_handles_[TABLE_DENTRY], Slice(keybuf, keylen));
    //delete inode
    txn->Delete(table_handles_[TABLE_INODE], Slice((char*)&ino, sizeof(ino)));

    //update parent inode: pino
    struct timespec curtime;
    clock_gettime(CLOCK_REALTIME, &curtime);
    parent_inoattr.nlink--;
    parent_inoattr.ctime = curtime;
    parent_inoattr.mtime = curtime;
    s = txn->Put(table_handles_[TABLE_INODE], Slice((char*)&pino, sizeof(pino)), Slice((char*)&parent_inoattr, sizeof(parent_inoattr)));

    s = txn->Commit();
    if (!s.ok()) {
        txn->Rollback();
        return RET_ERR;
    }
    delete txn;

    return RET_OK;
}


int MetaDB::Rename(uint64_t pino, const char *name, uint64_t newpino, const char *newname) {

    Transaction* txn = rocksdb_->BeginTransaction(write_options_, txn_options_);

    //get for update old dentry: pino+name
    int keylen = 0;
    char keybuf[MAX_DENTRY_KEY_LEN] = {0};
    EncodeDentryKey(pino, name, keybuf, &keylen);
    std::string tmp; 
    Status s = txn->GetForUpdate(read_options_, table_handles_[TABLE_DENTRY], Slice(keybuf, keylen), &tmp);
    if (s.IsNotFound()) {
        txn->Rollback();
        return RET_OK;
    }
    uint64_t ino = *(uint64_t*)tmp.data();

    //get for update old pinode:
    struct InodeAttr pino_inoattr = {0};
    s = txn->GetForUpdate(read_options_, table_handles_[TABLE_INODE], Slice((char*)&pino, sizeof(pino)), &tmp);
    if (!s.ok()) {
        txn->Rollback();
        return RET_ERR;
    }
    memcpy(&pino_inoattr, tmp.data(), tmp.size());

    //get for update new dentry: newpino + newname
    int new_keylen = 0;
    char new_keybuf[MAX_DENTRY_KEY_LEN] = {0};
    EncodeDentryKey(newpino, newname, new_keybuf, &new_keylen);
    s = txn->GetForUpdate(read_options_, table_handles_[TABLE_DENTRY], Slice(new_keybuf, new_keylen), &tmp);
    if (!s.IsNotFound()) {
        txn->Rollback();
        return RET_EEXIST;
    }

    //get for update new pinode:
    struct InodeAttr newpino_inoattr = {0};
    s = txn->GetForUpdate(read_options_, table_handles_[TABLE_INODE], Slice((char*)&newpino, sizeof(newpino)), &tmp);
    if (!s.ok()) {
        txn->Rollback();
        return RET_ERR;
    }
    memcpy(&newpino_inoattr, tmp.data(), tmp.size());


    //TODO: wrapper in funciton process later
    struct timespec curtime;
    clock_gettime(CLOCK_REALTIME, &curtime);

    struct InodeAttr inoattr = {0};
    s = txn->Get(read_options_, table_handles_[TABLE_INODE], Slice((char*)&ino, sizeof(ino)), &tmp);
    if (!s.ok()) {
        txn->Rollback();
        return RET_ERR;
    }
    memcpy(&inoattr, tmp.data(), tmp.size());


    //delete old dentry and insert new dentry;
    txn->Delete(table_handles_[TABLE_DENTRY], Slice(keybuf, keylen));
    txn->Put(table_handles_[TABLE_DENTRY], Slice(new_keybuf, new_keylen), Slice((char*)&ino, sizeof(ino)));

    //update oldpino and newpino attr
    pino_inoattr.ctime = curtime;
    pino_inoattr.mtime = curtime;
    newpino_inoattr.ctime = curtime;
    newpino_inoattr.mtime = curtime;
    if (S_ISDIR(inoattr.mode)) {
        pino_inoattr.nlink--;
        newpino_inoattr.nlink++;
    }
    txn->Put(table_handles_[TABLE_INODE], Slice((char*)&pino, sizeof(pino)), Slice((char*)&pino_inoattr, sizeof(pino_inoattr)));
    txn->Put(table_handles_[TABLE_INODE], Slice((char*)&newpino, sizeof(newpino)), Slice((char*)&newpino_inoattr, sizeof(newpino_inoattr)));

    s = txn->Commit();
    if (!s.ok()) {
        txn->Rollback();
        return RET_ERR;
    }
    delete txn;

    return RET_OK;

}

void MetaDB::RunBgTask() {
    
    uint64_t ino = 0;
    struct InodeAttr inoattr = {0};
    rocksdb::Iterator *iter = rocksdb_->NewIterator(read_options_, table_handles_[TABLE_TRASH]);
    iter->Seek(Slice((char*)&ino, sizeof(ino)));
    while (iter->Valid()) {
        ino = *(uint64_t*)(iter->key().data());
        char * valbuf = (char*)(iter->value().data());
        int vallen = iter->value().size();
        memcpy(&inoattr, valbuf, vallen);
        //delete file data
        pmeta_->DeleteFile(ino, inoattr.size);
        //delete file meta in transh
        rocksdb_->Delete(write_options_, table_handles_[TABLE_TRASH], iter->key());

        iter->Next();
    }
}







int MetaDB::PersistKV(const char*key, int klen, const char *value, int vlen) {
    Status s = rocksdb_->Put(write_options_, table_handles_[TABLE_DEFAULT], Slice(key, klen), Slice(value, vlen));
    if (s.ok()) {
        return RET_OK;
    } else {
        //print some error:
        return RET_ERR;
    }
}

int MetaDB::GetKV(const char *key, int klen, char *value, int *vlen) {
    std::string tmp;
    Status s = rocksdb_->Get(read_options_, table_handles_[TABLE_DEFAULT], Slice(key, klen), &tmp);
    if (s.ok()) {
        *vlen = tmp.size();
        memcpy(value, tmp.data(), *vlen);
        return RET_OK;
    } else if (s.IsNotFound()) {
        return RET_ENOENT;
    } else {
        return RET_ERR;
    }
}





void SeqAllocator::SeqInit(const char* seqname, MetaDB *ptr) {
    seq_id_ = 0;
    seq_id_persist_ = 0;
    dbptr_ = ptr;
    strcpy(seq_name_, seqname);

    //get seq_id_persist_;
    int vlen = 0;
    int ret = dbptr_->GetKV(seq_name_, strlen(seq_name_), (char*)seq_id_persist_, &vlen);
    if (ret == RET_ENOENT) {
        seq_id_ = SEQ_ID_BEGIN_;
    } else if (ret == RET_OK) {
        seq_id_ = seq_id_persist_;
    } else {
        exit(-1);
    }
    seq_id_persist_ = seq_id_ + SEQ_UPPER_STRIDE;

    ret = dbptr_->PersistKV(seq_name_, strlen(seq_name_), (char*)seq_id_persist_, sizeof(seq_id_persist_));
    if (ret != RET_OK) {
        exit(-1);
    }
}

uint64_t SeqAllocator::SeqGetone() {
    return seq_id_.fetch_add(1);
    /*
    if (seq_id_ < SEQ_ID_BEGIN_) {
        return 0;
    }

    seq_id_++;

    if (seq_id_ > seq_id_persist_) {
        //print some error; it will write to reserved inode 0;
        return 0;
    }
    
    if (seq_id_persist_ - seq_id_ < SEQ_LOWWER_STRIDE) {
        seq_id_persist_ += SEQ_UPPER_STRIDE;
        int ret = dbptr_->PersistKV(seq_name_, strlen(seq_name_), (char*)seq_id_persist_, sizeof(seq_id_persist_));
        if (ret != RET_OK) {
            //print some error;
        }
    }
    return seq_id_;
    */
}
