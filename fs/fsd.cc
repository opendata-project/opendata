
/* 
 * data process for file system daemon with Fuse and NFS/CIFS/S3 GATEWAY.
 * this daemon process can run both in applicaiton Node with fuse, 
 * or dedicated storage Node with NFS/CIFS
*/


#include <iostream>
#include <fstream>
#include <pthread.h>
#include "cfs_fuse.h"
#include "../util/log.h"
#include "cfs.h"

extern Cfs g_cfs_instance;


void* BgThreadSvc(void *arg) {

    pthread_t tid = pthread_self();
    SPDLOG_INFO("Background thread: tid={}", tid);

    bool runnning_flag = *(bool*)arg;
    while (runnning_flag) {
        sleep(10);
        g_cfs_instance.CfsRunBgTask();
    }
    return NULL;
}


int main(int argc, char *argv[]) {

	system("rm -rf /opt/cfs_meta_data/fsmeta_rocksdb");
    system("rm -rf /opt/cfs_meta_data/datastore/");
	system("rm -rf /opt/cfs_meta_data/test*.log*");
    
    InitLogUtil();
    g_cfs_instance.Init();
    SPDLOG_DEBUG("fsd init finish.!");

    //create background thread:
    pthread_t tid;
    bool running_flag = true;
    pthread_create(&tid, NULL, BgThreadSvc, &running_flag);
    

    int ret =  cfs_fuse_mainloop(argc, argv);
    //cfs_nfs_mainloop(argc, argv);
    //cfs_cifs_mainloop(argc, argv);
    //cfs_s3gw_mainloop(argc, argv);


    //join background thread:
    running_flag = false;
    pthread_join(tid, NULL);

    return 0;

}

