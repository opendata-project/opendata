/*
 *cluster file system interface implementation integrated with FUSE
*/

#include "cfs_fuse.h"
#include "cfs.h"

#include "fuse_kernel.h"


extern Cfs g_cfs_instance;

#define CFS_EOF -1


static const char *hello_str = "Hello World!\n";		//just for demo
static const char *hello_name = "hello";				//just for demo

static void inoattr_to_stat(struct InodeAttr *inoattr, struct stat *stbuf) {
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
	struct stat stbuf;
	(void)fi;
	memset(&stbuf, 0, sizeof(stbuf));
	struct InodeAttr inode_attr;
	memset(&inode_attr, 0, sizeof(inode_attr));

	if (g_cfs_instance.CfsGetattr(ino, &inode_attr) == -1) {
		fuse_reply_err(req, ENOENT);
	} else {
		inoattr_to_stat(&inode_attr, &stbuf);
		fuse_reply_attr(req, &stbuf, 1.0);
	}
}

static int hello_stat(fuse_ino_t ino, struct stat *stbuf)
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
	struct fuse_entry_param e;
	uint64_t ino;
	struct InodeAttr inoattr;
	int ret = g_cfs_instance.CfsLookup(parent, name, &ino, &inoattr);
	if (ret != RET_NO_ENTRY) {
		fuse_reply_err(req, ENOENT);
	} else {
		memset(&e, 0, sizeof(e));
		e.ino = ino;
		e.attr_timeout = 1.0;
		e.entry_timeout = 1.0;
		e.generation = 0;
		inoattr_to_stat(&inoattr, &e.attr);
		fuse_reply_entry(req, &e);
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

static void dirbuf_add(fuse_req_t req, struct dirbuf *b, const char *name,
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

static int reply_buf_limited(fuse_req_t req, const char *buf, size_t bufsize,
			     off_t off, size_t maxsize) {
	if (off < bufsize)
		return fuse_reply_buf(req, buf + off,
				      min(bufsize - off, maxsize));
	else
		return fuse_reply_buf(req, NULL, 0);
}

size_t fill_fuse_direntry(fuse_req_t req, char *buf, size_t bufsize, 
					std::vector<struct DirEntry*> *dentries, 
					std::vector<struct InodeAttr*> *inodes) {
	//if bufsize is not enough, should realloc buf.
	//off inside DirEntry be -1;
	return 0;
}

void cfs_fuse_readdir(fuse_req_t req, fuse_ino_t ino, size_t size,
			     off_t off, struct fuse_file_info *fi) {
	if (off == CFS_EOF) {
		fuse_reply_buf(req, NULL, 0);		//means EOF of directory
		return;
	}

	//TODO: actually we need support partial readdir
	//buf for simplicity now we just support one full readdir
	std::vector<struct DirEntry *> dentries;
	std::vector<struct InodeAttr *> inodes;
	int ret = g_cfs_instance.CfsReaddir(ino, off, &dentries, &inodes);
	if (ret == RET_OK) {
		size_t bufsize = dentries.size() * (sizeof(struct fuse_dirent) + FS_AVG_FILENAME_LEN);
		char * buf = (char*)malloc(bufsize);
		size_t buf_reallen = fill_fuse_direntry(req, buf, bufsize, &dentries, &inodes);
		fuse_reply_buf(req, buf, buf_reallen);
	} else {
		//dome some error handling;
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

void cfs_fuse_open(fuse_req_t req, fuse_ino_t ino,
			  struct fuse_file_info *fi) {
	if (ino != 2)
		fuse_reply_err(req, EISDIR);
	else if ((fi->flags & O_ACCMODE) != O_RDONLY)
		fuse_reply_err(req, EACCES);
	else
		fuse_reply_open(req, fi);

}

void cfs_fuse_read(fuse_req_t req, fuse_ino_t ino, size_t size,
			  off_t off, struct fuse_file_info *fi) {
	(void) fi;

	assert(ino == 2);
	reply_buf_limited(req, hello_str, strlen(hello_str), off, size);
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
	opts.foreground = true;		//add here for debug 2024-11-23
	opts.max_threads = 10;		//add here for debug 2024-11-23
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