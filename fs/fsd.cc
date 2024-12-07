
/* 
 * data process for file system daemon with Fuse and NFS/CIFS/S3 GATEWAY.
 * this daemon process can run both in applicaiton Node with fuse, 
 * or dedicated storage Node with NFS/CIFS
*/


#include <iostream>
#include <fstream>
#include "cfs_fuse.h"
#include "../util/log.h"
#include "cfs.h"

extern Cfs g_cfs_instance;





int main(int argc, char *argv[]) {

	system("rm -rf /opt/cfs_meta_data/fsmeta_rocksdb");
    system("rm -rf /opt/cfs_meta_data/datastore/");
	system("rm -rf /opt/cfs_meta_data/test*.log*");
    
    InitLogUtil();
    g_cfs_instance.Init();
    SPDLOG_DEBUG("fsd init finish.!");
    
    int ret =  cfs_fuse_mainloop(argc, argv);

    /* NFS and s3 gateway coming soon*/
    //cfs_nfs_mainloop(argc, argv);
    //cfs_cifs_mainloop(argc, argv);
    //cfs_s3gw_mainloop(argc, argv);

    //fout.close();
    return 0;

}

