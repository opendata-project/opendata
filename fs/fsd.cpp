
/* 
 * data process for file system daemon with Fuse and NFS/CIFS/S3 GATEWAY.
 * this daemon process can run both in applicaiton Node with fuse, 
 * or dedicated storage Node with NFS/CIFS
*/


#include "cfs_fuse.h"


int main(int argc, char *argv[])
{
    
    int ret =  cfs_fuse_mainloop(argc, argv);

    /* NFS and s3 gateway coming soon*/
    //cfs_nfs_mainloop(argc, argv);
    //cfs_cifs_mainloop(argc, argv);
    //cfs_s3gw_mainloop(argc, argv);

    return ret;

}
