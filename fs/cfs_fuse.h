/*
 *cluster file system interface integrated with FUSE
*/

#ifndef CFS_FUSE_H_
#define CFS_FUSE_H_

#ifdef __cplusplus
extern "C" {
#endif



#define FUSE_USE_VERSION 34

#include <fuse_lowlevel.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>


void cfs_fuse_init (void *userdata, struct fuse_conn_info *conn);
void cfs_fuse_destroy (void *userdata);
void cfs_fuse_lookup(fuse_req_t req, fuse_ino_t parent, const char *name);
void cfs_fuse_forget (fuse_req_t req, fuse_ino_t ino, uint64_t nlookup);
void cfs_fuse_getattr(fuse_req_t req, fuse_ino_t ino,
			     struct fuse_file_info *fi);
void cfs_fuse_setattr (fuse_req_t req, fuse_ino_t ino, struct stat *attr,
			 int to_set, struct fuse_file_info *fi);
void cfs_fuse_readlink (fuse_req_t req, fuse_ino_t ino);
void cfs_fuse_mknod (fuse_req_t req, fuse_ino_t parent, const char *name,
		       mode_t mode, dev_t rdev);
void cfs_fuse_mkdir (fuse_req_t req, fuse_ino_t parent, const char *name,
		       mode_t mode);
void cfs_fuse_unlink (fuse_req_t req, fuse_ino_t parent, const char *name);
void cfs_fuse_rmdir (fuse_req_t req, fuse_ino_t parent, const char *name);
void cfs_fuse_symlink (fuse_req_t req, const char *link, fuse_ino_t parent,
			 const char *name);
void cfs_fuse_rename (fuse_req_t req, fuse_ino_t parent, const char *name,
			fuse_ino_t newparent, const char *newname,
			unsigned int flags);
void cfs_fuse_link (fuse_req_t req, fuse_ino_t ino, fuse_ino_t newparent,
		      const char *newname);
void cfs_fuse_open(fuse_req_t req, fuse_ino_t ino,
			  struct fuse_file_info *fi);
void cfs_fuse_read(fuse_req_t req, fuse_ino_t ino, size_t size,
			  off_t off, struct fuse_file_info *fi);
void cfs_fuse_write (fuse_req_t req, fuse_ino_t ino, const char *buf,
		       size_t size, off_t off, struct fuse_file_info *fi);
void cfs_fuse_flush (fuse_req_t req, fuse_ino_t ino,
		       struct fuse_file_info *fi);
void cfs_fuse_release (fuse_req_t req, fuse_ino_t ino,
			 struct fuse_file_info *fi);
void cfs_fuse_fsync (fuse_req_t req, fuse_ino_t ino, int datasync,
		       struct fuse_file_info *fi);
void cfs_fuse_opendir (fuse_req_t req, fuse_ino_t ino,
			 struct fuse_file_info *fi);
void cfs_fuse_readdir(fuse_req_t req, fuse_ino_t ino, size_t size,
			     off_t off, struct fuse_file_info *fi);
void cfs_fuse_releasedir (fuse_req_t req, fuse_ino_t ino,
			    struct fuse_file_info *fi);
void cfs_fuse_fsyncdir (fuse_req_t req, fuse_ino_t ino, int datasync,
			  struct fuse_file_info *fi);
void cfs_fuse_statfs (fuse_req_t req, fuse_ino_t ino);
void cfs_fuse_setxattr (fuse_req_t req, fuse_ino_t ino, const char *name,
			  const char *value, size_t size, int flags);
void cfs_fuse_getxattr (fuse_req_t req, fuse_ino_t ino, const char *name,
			  size_t size);
void cfs_fuse_listxattr (fuse_req_t req, fuse_ino_t ino, size_t size);
void cfs_fuse_removexattr (fuse_req_t req, fuse_ino_t ino, const char *name);
void cfs_fuse_access (fuse_req_t req, fuse_ino_t ino, int mask);
void cfs_fuse_create (fuse_req_t req, fuse_ino_t parent, const char *name,
			mode_t mode, struct fuse_file_info *fi);
void cfs_fuse_getlk (fuse_req_t req, fuse_ino_t ino,
		       struct fuse_file_info *fi, struct flock *lock);
void cfs_fuse_setlk (fuse_req_t req, fuse_ino_t ino,
		       struct fuse_file_info *fi,
		       struct flock *lock, int sleep);
void cfs_fuse_bmap (fuse_req_t req, fuse_ino_t ino, size_t blocksize,
		      uint64_t idx);
#if FUSE_USE_VERSION < 35
	void cfs_fuse_ioctl (fuse_req_t req, fuse_ino_t ino, int cmd,
		       void *arg, struct fuse_file_info *fi, unsigned flags,
		       const void *in_buf, size_t in_bufsz, size_t out_bufsz);
#else
	void cfs_fuse_ioctl (fuse_req_t req, fuse_ino_t ino, unsigned int cmd,
		       void *arg, struct fuse_file_info *fi, unsigned flags,
		       const void *in_buf, size_t in_bufsz, size_t out_bufsz);
#endif
void cfs_fuse_poll (fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi,
		      struct fuse_pollhandle *ph);
void cfs_fuse_write_buf (fuse_req_t req, fuse_ino_t ino,
			   struct fuse_bufvec *bufv, off_t off,
			   struct fuse_file_info *fi);
void cfs_fuse_retrieve_reply (fuse_req_t req, void *cookie, fuse_ino_t ino,
				off_t offset, struct fuse_bufvec *bufv);
void cfs_fuse_forget_multi (fuse_req_t req, size_t count,
			      struct fuse_forget_data *forgets);
void cfs_fuse_flock (fuse_req_t req, fuse_ino_t ino,
		       struct fuse_file_info *fi, int op);
void cfs_fuse_fallocate (fuse_req_t req, fuse_ino_t ino, int mode,
		       off_t offset, off_t length, struct fuse_file_info *fi);
void cfs_fuse_readdirplus (fuse_req_t req, fuse_ino_t ino, size_t size, off_t off,
			 struct fuse_file_info *fi);
void cfs_fuse_copy_file_range (fuse_req_t req, fuse_ino_t ino_in,
				 off_t off_in, struct fuse_file_info *fi_in,
				 fuse_ino_t ino_out, off_t off_out,
				 struct fuse_file_info *fi_out, size_t len,
				 int flags);
void cfs_fuse_lseek (fuse_req_t req, fuse_ino_t ino, off_t off, int whence,
		       struct fuse_file_info *fi);


/*
* some test function
*/

void cfs_fuse_lookup_test(fuse_req_t req, fuse_ino_t parent, const char *name);
void cfs_fuse_getattr_test(fuse_req_t req, fuse_ino_t ino,
			     struct fuse_file_info *fi);
void cfs_fuse_readdir_test(fuse_req_t req, fuse_ino_t ino, size_t size,
			     off_t off, struct fuse_file_info *fi);
void cfs_fuse_open_test(fuse_req_t req, fuse_ino_t ino,
			  struct fuse_file_info *fi);
void cfs_fuse_read_test(fuse_req_t req, fuse_ino_t ino, size_t size,
			  off_t off, struct fuse_file_info *fi);
/*
* test function finish
*/

static const struct fuse_lowlevel_ops cfs_fuse_oper = {
    .init       = NULL,
    .destroy    = NULL,
    .lookup     = cfs_fuse_lookup, //cfs_fuse_lookup_test,
    .forget     = NULL,
    .getattr    = cfs_fuse_getattr, //cfs_fuse_getattr_test,
    .setattr    = cfs_fuse_setattr,
    .readlink   = NULL,
    .mknod      = cfs_fuse_mknod,
    .mkdir      = cfs_fuse_mkdir,
    .unlink     = cfs_fuse_unlink,
    .rmdir      = cfs_fuse_rmdir,
    .symlink    = NULL,
    .rename     = NULL,
    .link       = NULL,
	.open		= cfs_fuse_open, //cfs_fuse_open_test,
	.read		= cfs_fuse_read, //cfs_fuse_read_test,
    .write      = cfs_fuse_write,
    .flush      = NULL,
    .release    = cfs_fuse_release,
    .fsync      = NULL,
    .opendir    = NULL,
	.readdir	= cfs_fuse_readdir, //cfs_fuse_readdir_test,
    .releasedir = NULL,
    .fsyncdir   = NULL,
    .statfs     = NULL,
    .setxattr   = NULL,
    .getxattr   = NULL,
    .listxattr  = NULL,
    .removexattr    = NULL,
    .access     = NULL,
    .create     = cfs_fuse_create,
    .getlk      = NULL,
    .setlk      = NULL,
    .bmap       = NULL,
#if FUSE_USE_VERSION < 35
    .ioctl      = NULL,
#else
    .ioctl      = NULL,
#endif
    .poll       = NULL,
    .write_buf  = NULL,
    .retrieve_reply = NULL,
    .forget_multi   = NULL,
    .flock      = NULL,
    .fallocate  = NULL,
    .readdirplus    = cfs_fuse_readdirplus,
    .copy_file_range = NULL,
    .lseek      = NULL,
};



int cfs_fuse_mainloop(int argc, char *argv[]);


#ifdef __cplusplus
}
#endif

#endif  //FS_CFS_FUSE_H_