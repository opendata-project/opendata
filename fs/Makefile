
fsd:
	g++ -g -o ../build/fsd fsd.cc cfs_fuse.cc cfs.cc meta.cc meta_db.cc meta_cache.cc\
		/opt/open_source/rocksdb-8.11.4/librocksdb.a \
		-I/opt/open_source/libfuse-fuse-3.14.0/include \
		-I/opt/open_source/libfuse-fuse-3.14.0/lib \
		-I/opt/open_source/libfuse-fuse-3.14.0/build \
		-I/opt/open_source/rocksdb-8.11.4/include \
		-L/opt/open_source/libfuse-fuse-3.14.0/build/lib \
		-lfuse3

mds:
	g++ -g -o ../build/mds mds.cc \
		/opt/open_source/rocksdb-8.11.4/librocksdb.a \
		-I/opt/open_source/rocksdb-8.11.4/include \

test:
	
clean:
	rm -rf ../build/*