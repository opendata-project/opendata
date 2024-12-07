/*
 *cluster file system interface implementation integrated with FUSE
*/

#include "cfs_fuse.h"
#include "fuse_kernel.h"
#include "cfs.h"
#include "defines.h"
#include "../util/log.h"


extern Cfs g_cfs_instance;


const char *hello_str = "Hello World!\n";		//just for demo
const char *hello_name = "hello";				//just for demo

void inoattr_to_stat(struct InodeAttr *inoattr, struct stat *stbuf) {
	stbuf->st_atim = inoattr->atime;
	stbuf->st_ctim = inoattr->ctime;
	stbuf->st_gid = inoattr->gid;
	stbuf->st_ino = inoattr->ino;
	stbuf->st_mode = inoattr->mode;
	stbuf->st_mtim = inoattr->mtime;
	stbuf->st_nlink = inoattr->nlink;
	stbuf->st_size = inoattr->size;
	stbuf->st_uid = inoattr->uid;
}

void cfs_fuse_getattr(fuse_req_t req, fuse_ino_t ino,
			     struct fuse_file_info *fi) {
	SPDLOG_DEBUG("Enter cfs_fuse_getattr: ino={}", ino);
	struct stat stbuf;
	(void)fi;
	memset(&stbuf, 0, sizeof(stbuf));
	struct InodeAttr inode_attr = {0};

	if (g_cfs_instance.CfsGetattr(ino, &inode_attr) == -1) {
		fuse_reply_err(req, ENOENT);
	} else {
		inoattr_to_stat(&inode_attr, &stbuf);
		fuse_reply_attr(req, &stbuf, 1.0);
	}
}

void cfs_fuse_setattr (fuse_req_t req, fuse_ino_t ino, struct stat *attr,
			 int to_set, struct fuse_file_info *fi) {
	SPDLOG_DEBUG("Enter cfs_fuse_setattr: ino={}, to_set={0:b}", ino, to_set);
	struct InodeAttr inoattr = {0};
	int setmask = 0;

	if (to_set & FUSE_SET_ATTR_MODE) {
		setmask |= CFS_SET_ATTR_MODE;
		inoattr.mode = attr->st_mode;
	}
	if (to_set & FUSE_SET_ATTR_UID) {
		setmask |= CFS_SET_ATTR_UID;
		inoattr.uid = attr->st_uid;
	}
	if (to_set & FUSE_SET_ATTR_GID) {
		setmask |= CFS_SET_ATTR_GID;
		inoattr.gid = attr->st_gid;
	}
	if (to_set & FUSE_SET_ATTR_ATIME) {
		setmask |= CFS_SET_ATTR_ATIME;
		inoattr.atime = attr->st_atim;
	}
	if (to_set & FUSE_SET_ATTR_MTIME) {
		setmask |= CFS_SET_ATTR_MTIME;
		inoattr.mtime = attr->st_mtim;
	}
	if (to_set & FUSE_SET_ATTR_ATIME_NOW) {
		setmask |= CFS_SET_ATTR_ATIME_NOW;
	}
	if (to_set & FUSE_SET_ATTR_MTIME_NOW) {
		setmask |= CFS_SET_ATTR_MTIME_NOW;
	}
	if (to_set & FUSE_SET_ATTR_CTIME) {
		setmask |= CFS_SET_ATTR_CTIME;
		inoattr.ctime = attr->st_ctim;
	}
	if (to_set & FUSE_SET_ATTR_SIZE) {
		setmask |= CFS_SET_ATTR_SIZE;
		inoattr.size = attr->st_size;
	}
	if (fi != NULL) {
		setmask |= CFS_SET_ATTR_TRUNCATE;
	}


	struct InodeAttr new_inoattr = {0};
	if (g_cfs_instance.CfsSetattr(ino, &inoattr, setmask, &new_inoattr) == -1) {
		fuse_reply_err(req, EAGAIN);
	} else {
		inoattr_to_stat(&new_inoattr, attr);
		fuse_reply_attr(req, attr, 1.0);
	}
}


int hello_stat(fuse_ino_t ino, struct stat *stbuf)
{
	stbuf->st_ino = ino;
	switch (ino) {
	case 1:
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
		break;

	case 2:
		stbuf->st_mode = S_IFREG | 0444;
		stbuf->st_nlink = 1;
		stbuf->st_size = strlen(hello_str);
		break;

	default:
		return -1;
	}
	return 0;
}

void cfs_fuse_getattr_test(fuse_req_t req, fuse_ino_t ino,
			     struct fuse_file_info *fi) {
	struct stat stbuf;

	(void) fi;

	memset(&stbuf, 0, sizeof(stbuf));
	if (hello_stat(ino, &stbuf) == -1)
		fuse_reply_err(req, ENOENT);
	else
		fuse_reply_attr(req, &stbuf, 1.0);
}

void cfs_fuse_lookup(fuse_req_t req, fuse_ino_t parent, const char *name) {
	SPDLOG_INFO("Enter cfs_fuse_lookup: pino={}, name={}", parent, name);
	struct fuse_entry_param e;
	uint64_t ino = 0;
	struct InodeAttr inoattr = {0};
	int ret = 0;
	if (0 == strcmp(name, ".")) {
		ret = g_cfs_instance.CfsGetattr(parent, &inoattr);
		if (ret != RET_OK) {
			SPDLOG_ERROR("CFS GETATTR OF name='.' error: {}.", ret);
			fuse_reply_err(req, ENOENT);
		} 
	} else {
		ret = g_cfs_instance.CfsLookup(parent, name, &ino, &inoattr);
		if (ret == RET_ENOENT) {
			fuse_reply_err(req, ENOENT);
		} else if (ret == RET_OK) {
			memset(&e, 0, sizeof(e));
			e.ino = ino;
			e.attr_timeout = 1.0;
			e.entry_timeout = 1.0;
			e.generation = 0;
			inoattr_to_stat(&inoattr, &e.attr);
			fuse_reply_entry(req, &e);
		} else {
			fuse_reply_err(req, EBUSY);
		}
	}
}

void cfs_fuse_lookup_test(fuse_req_t req, fuse_ino_t parent, const char *name) {
	struct fuse_entry_param e;

	if (parent != 1 || strcmp(name, hello_name) != 0)
		fuse_reply_err(req, ENOENT);
	else {
		memset(&e, 0, sizeof(e));
		e.ino = 2;
		e.attr_timeout = 1.0;
		e.entry_timeout = 1.0;
		hello_stat(e.ino, &e.attr);

		fuse_reply_entry(req, &e);
	}
}

struct dirbuf {
	char *p;
	size_t size;
};

void dirbuf_add(fuse_req_t req, struct dirbuf *b, const char *name,
		       fuse_ino_t ino) {
	struct stat stbuf;
	size_t oldsize = b->size;
	b->size += fuse_add_direntry(req, NULL, 0, name, NULL, 0);
	b->p = (char *) realloc(b->p, b->size);
	memset(&stbuf, 0, sizeof(stbuf));
	stbuf.st_ino = ino;
	fuse_add_direntry(req, b->p + oldsize, b->size - oldsize, name, &stbuf,
			  b->size);
}


#define min(x, y) ((x) < (y) ? (x) : (y))

int reply_buf_limited(fuse_req_t req, const char *buf, size_t bufsize,
			     off_t off, size_t maxsize) {
	if (off < bufsize)
		return fuse_reply_buf(req, buf + off,
				      min(bufsize - off, maxsize));
	else
		return fuse_reply_buf(req, NULL, 0);
}

size_t fill_fuse_direntry(fuse_req_t req, char *buf, size_t bufsize, 
					std::vector<struct DirEntry*> *dentries) {
	//TODO： if bufsize is not enough, should realloc buf.
	//TODO： off inside DirEntry be -1;
	struct stat stbuf = {0};
	int size = 0;
	for (int i = 0; i < dentries->size(); i++) {
		size = fuse_add_direntry(req, NULL, 0, (*dentries)[i]->name, NULL, 0);	//just get size of entry
		assert(bufsize > size);
		if (bufsize < size) {
			//TODO: realloc a big size of buf;
		}
		stbuf.st_ino = (*dentries)[i]->ino;
		stbuf.st_mode = 0; 														//equal fuse kernel DT_UNKOWN=0
		fuse_add_direntry(req, buf, bufsize, (*dentries)[i]->name, &stbuf, CFS_EOF);
		buf += size;
		bufsize -= size;
	}
	return bufsize;
}

void cfs_fuse_readdir(fuse_req_t req, fuse_ino_t ino, size_t size,
			     off_t off, struct fuse_file_info *fi) {
	SPDLOG_INFO("Enter cfs_fuse_readdir: ino={}, size={}, off={}", ino, size, off);
	if (off == CFS_EOF) {
		fuse_reply_buf(req, NULL, 0);		//means EOF of directory
		return;
	}

	//TODO: actually we need support partial readdir
	//buf for simplicity now we just support one full readdir
	std::vector<struct DirEntry *> dentries;
	std::vector<struct InodeAttr *> inodes;
	int ret = g_cfs_instance.CfsReaddir(ino, off, &dentries, &inodes, false);
	if (ret == RET_OK) {
		size_t bufsize = dentries.size() * (sizeof(struct fuse_dirent) + CFS_AVG_FILENAME_LEN);
		char * buf = (char*)malloc(bufsize);
		size_t buf_left = fill_fuse_direntry(req, buf, bufsize, &dentries);
		fuse_reply_buf(req, buf, bufsize - buf_left);
		free(buf);
	} else {
		//dome some error handling;
	}

	//free dentries and inodes memory
	for (int i = 0; i < dentries.size(); i++) {
		delete dentries[i];
	}
	for (int i = 0; i < inodes.size(); i++) {

		delete inodes[i];
	}
}

size_t fill_fuse_direntry_plus(fuse_req_t req, char *buf, size_t bufsize, 
					std::vector<struct DirEntry*> *dentries, 
					std::vector<struct InodeAttr*> *inodes) {
	//TODO： if bufsize is not enough, should realloc buf.
	//TODO： off inside DirEntry be -1;
	int size = 0;
	for (int i = 0; i < dentries->size(); i++) {
		size = fuse_add_direntry_plus(req, NULL, 0, (*dentries)[i]->name, NULL, 0);	//just get size of entry
		assert(bufsize > size);
		if (bufsize < size) {
			//TODO: realloc a big size of buf;
		}
		
		struct fuse_entry_param entryparam = {0};
		inoattr_to_stat((*inodes)[i], &entryparam.attr);
		entryparam.ino = (*dentries)[i]->ino;
		entryparam.generation = 0;
		entryparam.attr_timeout = 1.0;
		entryparam.entry_timeout = 1.0;
		fuse_add_direntry_plus(req, buf, bufsize, (*dentries)[i]->name, &entryparam, CFS_EOF);
		buf += size;
		bufsize -= size;
	}
	return bufsize;
}

void cfs_fuse_readdirplus (fuse_req_t req, fuse_ino_t ino, size_t size, off_t off,
			 struct fuse_file_info *fi) {
	SPDLOG_INFO("Enter cfs_fuse_readdirplus: ino={}, size={}, off={}", ino, size, off);
	if (off == CFS_EOF) {
		fuse_reply_buf(req, NULL, 0);		//means EOF of directory
		return;
	}

	//TODO: actually we need support partial readdir
	//buf for simplicity now we just support one full readdir
	std::vector<struct DirEntry *> dentries;
	std::vector<struct InodeAttr *> inodes;
	int ret = g_cfs_instance.CfsReaddir(ino, off, &dentries, &inodes, true);
	if (ret == RET_OK) {
		size_t bufsize = dentries.size() * (sizeof(struct fuse_direntplus) + CFS_AVG_FILENAME_LEN);
		char * buf = (char*)malloc(bufsize);
		size_t buf_left = fill_fuse_direntry_plus(req, buf, bufsize, &dentries, &inodes);
		fuse_reply_buf(req, buf, bufsize - buf_left);
	} else {
		//dome some error handling;
	}

	//free dentries and inodes
	for (int i = 0; i < dentries.size(); i++) {
		delete dentries[i];
	}
	for (int i = 0; i < inodes.size(); i++) {
		delete inodes[i];
	}
}


void cfs_fuse_readdir_test(fuse_req_t req, fuse_ino_t ino, size_t size,
			     off_t off, struct fuse_file_info *fi) {
	(void) fi;

	if (ino != 1)
		fuse_reply_err(req, ENOTDIR);
	else {
		struct dirbuf b;

		memset(&b, 0, sizeof(b));
		dirbuf_add(req, &b, ".", 1);
		dirbuf_add(req, &b, "..", 1);
		dirbuf_add(req, &b, hello_name, 2);
		reply_buf_limited(req, b.p, b.size, off, size);
		free(b.p);
	}
}


//fuse create and open file
void cfs_fuse_create (fuse_req_t req, fuse_ino_t parent, const char *name,
			mode_t mode, struct fuse_file_info *fi) {
	SPDLOG_INFO("Enter cfs_fuse_create: parent={}, name={}, mode={0:o}", parent, name, mode);
	struct InodeAttr inoattr = {0};
	inoattr.uid = fuse_req_ctx(req)->uid;
	inoattr.gid = fuse_req_ctx(req)->gid;
    //inoattr.ino = ?;
    inoattr.pino = parent;
	inoattr.mode = mode;
    inoattr.nlink = (S_ISDIR(mode) ? 2 : 1);	
	inoattr.size = (S_ISDIR(mode) ? 4096 : 0);
	struct timespec curtime;
    clock_gettime(CLOCK_REALTIME, &curtime);
    inoattr.atime = curtime;
    inoattr.ctime = curtime;
    inoattr.mtime = curtime;

	int ret = g_cfs_instance.CfsCreate(parent, name, mode, &inoattr);
	if (ret != RET_OK) {
		//print some error
	}

	
	ret = g_cfs_instance.CfsOpen(inoattr.ino, fi->flags, &fi->fh);
	if (ret != RET_OK) {
		//print some error
	}

	fuse_entry_param entry_param = {0};
	inoattr_to_stat(&inoattr, &entry_param.attr);
	entry_param.attr_timeout = 1.0;
	entry_param.entry_timeout = 1.0;
	entry_param.generation = 0;
	entry_param.ino = inoattr.ino;

	fuse_reply_create(req, &entry_param, fi);

}

//mknod and mkdir (not supoort mk dev)
void cfs_fuse_mknod (fuse_req_t req, fuse_ino_t parent, const char *name,
		       mode_t mode, dev_t rdev) {
	SPDLOG_DEBUG("Enter cfs_fuse_mknod: parent={}, name={}, mode={0:o}", parent, name, mode);
	struct InodeAttr inoattr = {0};
	inoattr.uid = fuse_req_ctx(req)->uid;
	inoattr.gid = fuse_req_ctx(req)->gid;
    //inoattr.ino = ?;
    inoattr.pino = parent;
	inoattr.mode = mode;
    inoattr.nlink = (S_ISDIR(mode) ? 2 : 1);	
	inoattr.size = (S_ISDIR(mode) ? 4096 : 0);
	struct timespec curtime;
    clock_gettime(CLOCK_REALTIME, &curtime);
    inoattr.atime = curtime;
    inoattr.ctime = curtime;
    inoattr.mtime = curtime;

	int ret = g_cfs_instance.CfsCreate(parent, name, mode, &inoattr);
	if (ret != RET_OK) {
		//print some error
	}

	fuse_entry_param entry_param = {0};
	inoattr_to_stat(&inoattr, &entry_param.attr);
	entry_param.attr_timeout = 1.0;
	entry_param.entry_timeout = 1.0;
	entry_param.generation = 0;
	entry_param.ino = inoattr.ino;

	fuse_reply_entry(req, &entry_param);
}

void cfs_fuse_mkdir (fuse_req_t req, fuse_ino_t parent, const char *name,
		       mode_t mode) {
	SPDLOG_INFO("Enter cfs_fuse_mkdir: parent={}, name={}, mode={0:o}", parent, name, mode);
	struct InodeAttr inoattr = {0};
	inoattr.uid = fuse_req_ctx(req)->uid;
	inoattr.gid = fuse_req_ctx(req)->gid;
    //inoattr.ino = ?;
    inoattr.pino = parent;
	mode |= S_IFDIR; 		//i don't know why input param mode is 775 with S_IFDIR bits 0;
	inoattr.mode = mode;
    inoattr.nlink = (S_ISDIR(mode) ? 2 : 1);	
	inoattr.size = (S_ISDIR(mode) ? 4096 : 0);
	struct timespec curtime;
    clock_gettime(CLOCK_REALTIME, &curtime);
    inoattr.atime = curtime;
    inoattr.ctime = curtime;
    inoattr.mtime = curtime;

	int ret = g_cfs_instance.CfsCreate(parent, name, mode, &inoattr);
	if (ret != RET_OK) {
		//print some error
	}

	fuse_entry_param entry_param = {0};
	inoattr_to_stat(&inoattr, &entry_param.attr);
	entry_param.attr_timeout = 1.0;
	entry_param.entry_timeout = 1.0;
	entry_param.generation = 0;
	entry_param.ino = inoattr.ino;

	fuse_reply_entry(req, &entry_param);
}


void cfs_fuse_unlink (fuse_req_t req, fuse_ino_t parent, const char *name) {
	SPDLOG_DEBUG("Enter cfs_fuse_unlink: parent={}, name={}", parent, name);
	g_cfs_instance.CfsUnlink(parent, name);
	fuse_reply_err(req, 0);
}

void cfs_fuse_rmdir (fuse_req_t req, fuse_ino_t parent, const char *name) {
	SPDLOG_DEBUG("Enter cfs_fuse_rmdir: parent={}, name={}");
	int ret = g_cfs_instance.CfsRmdir(parent, name, true);
	if (ret == RET_ENOTDIR) {
		fuse_reply_err(req, ENOTDIR);
	}
	fuse_reply_err(req, 0);
}


//fuse only open file
void cfs_fuse_open(fuse_req_t req, fuse_ino_t ino,
			  struct fuse_file_info *fi) {
	SPDLOG_DEBUG("Enter cfs_fuse_open: ino={}", ino);
	int ret = g_cfs_instance.CfsOpen(ino, fi->flags, &fi->fh);
	if (ret != RET_OK) {
		//print some error
		//fuse_reply_err
	}
	fuse_reply_open(req, fi);
}


void cfs_fuse_release (fuse_req_t req, fuse_ino_t ino,
			 struct fuse_file_info *fi) {
	SPDLOG_DEBUG("Enter cfs_fuse_release: ino={}", ino);
	int ret = g_cfs_instance.CfsRelease(ino, fi->fh);
	if (ret != RET_OK) {
		//print some error.
		//fuse_reply_err
	}
	fuse_reply_err(req, 0);

}



void cfs_fuse_open_test(fuse_req_t req, fuse_ino_t ino,
			  struct fuse_file_info *fi) {
	if (ino != 2)
		fuse_reply_err(req, EISDIR);
	else if ((fi->flags & O_ACCMODE) != O_RDONLY)
		fuse_reply_err(req, EACCES);
	else
		fuse_reply_open(req, fi);

}


void cfs_fuse_read_test(fuse_req_t req, fuse_ino_t ino, size_t size,
			  off_t off, struct fuse_file_info *fi) {
	(void) fi;

	assert(ino == 2);
	reply_buf_limited(req, hello_str, strlen(hello_str), off, size);
}

void cfs_fuse_read(fuse_req_t req, fuse_ino_t ino, size_t size,
			  off_t off, struct fuse_file_info *fi) {
	SPDLOG_DEBUG("Enter cfs_fuse_read: ino={}, size={}, off={}", ino, size, off);
	if (size <= 0 || size > CFS_MAX_BLK_SIZE) {
		fuse_reply_err(req, EINVAL);
	}
	char *rdbuf = (char*)malloc(size);
	int cnt = g_cfs_instance.CfsRead(ino, fi->fh, size, off, rdbuf);
	if (cnt == RET_ERR || cnt > size) {
		//print some error;
		fuse_reply_err(req,EINVAL);
	}
	
	fuse_reply_buf(req, rdbuf, cnt);

	free(rdbuf);
} 

void cfs_fuse_write (fuse_req_t req, fuse_ino_t ino, const char *buf,
		       size_t size, off_t off, struct fuse_file_info *fi) {
	SPDLOG_DEBUG("Enter cfs_fuse_write: ino={}, size={}, off={}", ino, size, off);
	if (size <= 0 || size > CFS_MAX_BLK_SIZE) {
		fuse_reply_err(req, EINVAL);
	}

	int cnt = g_cfs_instance.CfsWrite(ino, fi->fh, size, off, (char*)buf);
	if (cnt == RET_ERR || cnt > size) {
		//print some error;
		fuse_reply_err(req,EINVAL);
	}
	fuse_reply_write(req, cnt);
}


int cfs_fuse_mainloop(int argc, char *argv[]) {
	struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
	struct fuse_session *se;
	struct fuse_cmdline_opts opts;
	struct fuse_loop_config config;
	int ret = -1;

	if (fuse_parse_cmdline(&args, &opts) != 0)
		return 1;
	if (opts.show_help) {
		printf("usage: %s [options] <mountpoint>\n\n", argv[0]);
		fuse_cmdline_help();
		fuse_lowlevel_help();
		ret = 0;
		goto err_out1;
	} else if (opts.show_version) {
		printf("FUSE library version %s\n", fuse_pkgversion());
		fuse_lowlevel_version();
		ret = 0;
		goto err_out1;
	}

	if(opts.mountpoint == NULL) {
		printf("usage: %s [options] <mountpoint>\n", argv[0]);
		printf("       %s --help\n", argv[0]);
		ret = 1;
		goto err_out1;
	}

	se = fuse_session_new(&args, &cfs_fuse_oper,
			      sizeof(cfs_fuse_oper), NULL);
	if (se == NULL)
	    goto err_out1;

	if (fuse_set_signal_handlers(se) != 0)
	    goto err_out2;

	if (fuse_session_mount(se, opts.mountpoint) != 0)
	    goto err_out3;
	opts.foreground = true;
	opts.max_threads = 1;		//add here for debug 2024-11-23
	fuse_daemonize(opts.foreground);

	/* Block until ctrl+c or fusermount -u */
	if (opts.singlethread)
		ret = fuse_session_loop(se);
	else {
		config.clone_fd = opts.clone_fd;
		config.max_idle_threads = opts.max_idle_threads;
		ret = fuse_session_loop_mt(se, &config);
	}

	fuse_session_unmount(se);
err_out3:
	fuse_remove_signal_handlers(se);
err_out2:
	fuse_session_destroy(se);
err_out1:
	free(opts.mountpoint);
	fuse_opt_free_args(&args);

	return ret ? 1 : 0;
}