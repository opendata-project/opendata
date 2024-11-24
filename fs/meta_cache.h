/*
* implement metadata cache, that can be located in mds process or fsd process
*/

#ifndef FS_META_CACHE_H_
#define FS_META_CACHE_H_


class MetaCache {
public:
    MetaCache();
    ~MetaCache();
    void Init();
    void Finish();

private:

};


#endif