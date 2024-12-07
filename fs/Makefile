
INCLUDE=-I/opt/open_source/libfuse-fuse-3.14.0/include \
		-I/opt/open_source/libfuse-fuse-3.14.0/build \
		-I/opt/open_source/rocksdb-8.11.4/include \
		-I/opt/open_source/spdlog-1.14.1/include

NOTUSED=-I/opt/open_source/libfuse-fuse-3.14.0/lib


DLIBPATH=-L/opt/open_source/libfuse-fuse-3.14.0/build/lib
DLIB=-lfuse3
SLIB=/opt/open_source/rocksdb-8.11.4/librocksdb.a \
		/opt/open_source/spdlog-1.14.1/libspdlog.a


cfs_fuse.o : cfs_fuse.cc
	g++ -g -c cfs_fuse.cc -o ../build/cfs_fuse.o $(INCLUDE) 
cfs.o : cfs.cc
	g++ -g -c cfs.cc -o ../build/cfs.o $(INCLUDE)
meta.o : meta.cc
	g++ -g -c meta.cc -o ../build/meta.o $(INCLUDE)
data.o : data.cc
	g++ -g -c data.cc -o ../build/data.o $(INCLUDE)
meta_db.o : meta_db.cc
	g++ -g -c meta_db.cc -o ../build/meta_db.o $(INCLUDE)
meta_cache.o : meta_cache.cc
	g++ -g -c meta_cache.cc -o ../build/meta_cache.o $(INCLUDE)
fsd.o : fsd.cc
	g++ -g -c fsd.cc -o ../build/fsd.o $(INCLUDE)
log.o : ../util/log.cc
	g++ -g -c ../util/log.cc -o ../build/log.o $(INCLUDE)
mds.o : mds.cc
	g++ -g -c mds.cc -o ../build/mds.o $(INCLUDE)

fsd : fsd.o cfs_fuse.o cfs.o meta.o data.o meta_db.o meta_cache.o log.o
	g++ -g ../build/fsd.o ../build/cfs_fuse.o ../build/cfs.o ../build/meta.o ../build/data.o ../build/meta_db.o \
			../build/meta_cache.o  ../build/log.o -o ../build/fsd \
			$(INCLUDE) $(SLIB) $(DLIBPATH) $(DLIB)

mds : mds.o log.o
	g++ -g ../build/mds.o ../build/log.o -o ../build/mds $(INCLUDE) $(SLIB) $(DLIBPATH) $(DLIB)

tmp1:
	g++ -g -o ../build/fsd ../util/log.cc \
		fsd.cc cfs_fuse.cc cfs.cc meta.cc data.cc meta_db.cc meta_cache.cc \
		-I/opt/open_source/libfuse-fuse-3.14.0/include \
		-I/opt/open_source/libfuse-fuse-3.14.0/lib \
		-I/opt/open_source/libfuse-fuse-3.14.0/build \
		-I/opt/open_source/rocksdb-8.11.4/include \
		-I/opt/open_source/spdlog-1.14.1/include \
		/opt/open_source/rocksdb-8.11.4/librocksdb.a \
		/opt/open_source/spdlog-1.14.1/libspdlog.a \
		-L/opt/open_source/libfuse-fuse-3.14.0/build/lib \
		-lfuse3

tmp2:
	g++ -g -o ../build/mds ../util/log.cc \
		mds.cc  \
		-I/opt/open_source/rocksdb-8.11.4/include \
		-I/opt/open_source/spdlog-1.14.1/include \
		/opt/open_source/rocksdb-8.11.4/librocksdb.a \
		/opt/open_source/spdlog-1.14.1/libspdlog.a \

test:
	
clean:
	rm -rf ../build/*