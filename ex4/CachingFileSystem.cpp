/*
 * CachingFileSystem.cpp
 *
 *  Created on: 15 April 2015
 *  Author: Netanel Zakay, HUJI, 67808  (Operating Systems 2014-2015).
 */

#define FUSE_USE_VERSION 26

//todo: restore this
//#include <fuse.h>
//todo: remove this
#include <osxfuse/fuse.h>
#include "CacheState.h"
#include "Logger.h"
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <dirent.h>

using namespace std;

struct fuse_operations caching_oper;


static const int FAILURE = -1;
static const int SYS_ERROR = 1;
static const char *const LOGGER_FILENAME = "/.filesystem.log";
static const char *const SYSTEM_ERROR_PREFIX = "System Error: "
static const char *const ROOTDIR_ERROR_MSG = "Cannot convert rootDir to absolute path";
static const char *const ROOTDIR_TOO_LONG_ERROR_MSG = "rootDir is to long to create the logger file";
static const char *const OPEN_LOGGER_ERROR_MSG = "Cannot open logger file";
static const char *const USAGE = "usage: CachingFileSystem rootdir mountdir numberOfBlocks "
        "blockSize";

static const char *const CACHE_ALLOC_ERROR_MSG = "Cannot allocate the desired space for cache";

static const char *const INIT_FUNC = "init";

static const char *const DESTROY_FUNC = "destroy";

static const char *const RELEASEDIR_FUNC = "releasedir";

static const int SUCCESS = 0;

void handleSystemError(const char *msg)
{
    std::cerr << SYSTEM_ERROR_PREFIX << msg << std::endl;
    exit(SYS_ERROR);
}

/** Get file attributes.
 *
 * Similar to stat().  The 'st_dev' and 'st_blksize' fields are
 * ignored.  The 'st_ino' field is ignored except if the 'use_ino'
 * mount option is given.
 */
int caching_getattr(const char *path, struct stat *statbuf)
{
    return 0;
}

/**
 * Get attributes from an open file
 *
 * This method is called instead of the getattr() method if the
 * file information is available.
 *
 * Currently this is only called after the create() method if that
 * is implemented (see above).  Later it may be called for
 * invocations of fstat() too.
 *
 * Introduced in version 2.5
 */
int caching_fgetattr(const char *path, struct stat *statbuf, struct fuse_file_info *fi)
{
    return 0;
}

/**
 * Check file access permissions
 *
 * This will be called for the access() system call.  If the
 * 'default_permissions' mount option is given, this method is not
 * called.
 *
 * This method is not called under Linux kernel versions 2.4.x
 *
 * Introduced in version 2.5
 */
int caching_access(const char *path, int mask)
{
    return 0;
}


/** File open operation
 *
 * No creation, or truncation flags (O_CREAT, O_EXCL, O_TRUNC)
 * will be passed to open().  Open should check if the operation
 * is permitted for the given flags.  Optionally open may also
 * return an arbitrary filehandle in the fuse_file_info structure,
 * which will be passed to all file operations.

 * pay attention that the max allowed path is PATH_MAX (in limits.h).
 * if the path is longer, return error.

 * Changed in version 2.2
 */
int caching_open(const char *path, struct fuse_file_info *fi)
{
    return 0;
}


/** Read data from an open file
 *
 * Read should return exactly the number of bytes requested except
 * on EOF or error, otherwise the rest of the data will be
 * substituted with zeroes. 
 *
 * Changed in version 2.2
 */
int caching_read(const char *path, char *buf, size_t size, off_t offset,
                 struct fuse_file_info *fi)
{
    return 0;
}

/** Possibly flush cached data
 *
 * BIG NOTE: This is not equivalent to fsync().  It's not a
 * request to sync dirty data.
 *
 * Flush is called on each close() of a file descriptor.  So if a
 * filesystem wants to return write errors in close() and the file
 * has cached dirty data, this is a good place to write back data
 * and return any errors.  Since many applications ignore close()
 * errors this is not always useful.
 *
 * NOTE: The flush() method may be called more than once for each
 * open().  This happens if more than one file descriptor refers
 * to an opened file due to dup(), dup2() or fork() calls.  It is
 * not possible to determine if a flush is final, so each flush
 * should be treated equally.  Multiple write-flush sequences are
 * relatively rare, so this shouldn't be a problem.
 *
 * Filesystems shouldn't assume that flush will always be called
 * after some writes, or that if will be called at all.
 *
 * Changed in version 2.2
 */
int caching_flush(const char *path, struct fuse_file_info *fi)
{
    return 0;
}

/** Release an open file
 *
 * Release is called when there are no more references to an open
 * file: all file descriptors are closed and all memory mappings
 * are unmapped.
 *
 * For every open() call there will be exactly one release() call
 * with the same flags and file descriptor.  It is possible to
 * have a file opened more than once, in which case only the last
 * release will mean, that no more reads/writes will happen on the
 * file.  The return value of release is ignored.
 *
 * Changed in version 2.2
 */
int caching_release(const char *path, struct fuse_file_info *fi)
{
    return 0;
}

/** Open directory
 *
 * This method should check if the open operation is permitted for
 * this  directory
 *
 * Introduced in version 2.3
 */
int caching_opendir(const char *path, struct fuse_file_info *fi)
{
    return 0;
}

/** Read directory
 *
 * This supersedes the old getdir() interface.  New applications
 * should use this.
 *
 * The readdir implementation ignores the offset parameter, and
 * passes zero to the filler function's offset.  The filler
 * function will not return '1' (unless an error happens), so the
 * whole directory is read in a single readdir operation.  This
 * works just like the old getdir() method.
 *
 * Introduced in version 2.3
 */
int caching_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset,
                    struct fuse_file_info *fi)
{
    return 0;
}

/** Release directory
 *
 * Introduced in version 2.3
 */
int caching_releasedir(const char *path, struct fuse_file_info *fi)
{
    if (logFunctionEntry(RELEASEDIR_FUNC) < 0)
    {
        return -errno;
    }
    
    if (closedir( (DIR *) (uintptr_t) fi->fh) == FAILURE)
    {
        return -errno;
    }
    
    return SUCCESS;
}

/** Rename a file */
int caching_rename(const char *path, const char *newpath)
{
    return 0;
}

/**
 * Initialize filesystem
 *
 * The return value will passed in the private_data field of
 * fuse_context to all file operations and as a parameter to the
 * destroy() method.
 *
 * Introduced in version 2.3
 * Changed in version 2.6
 */
void *caching_init(struct fuse_conn_info *conn)
{
    logFunctionEntry(INIT_FUNC);

    return STATE;
}


/**
 * Clean up filesystem
 *
 * Called on filesystem exit.
 *
 * Introduced in version 2.3
 */
void caching_destroy(void *userdata)
{
    logFunctionEntry(DESTROY_FUNC);
}


/**
 * Ioctl from the FUSE sepc:
 * flags will have FUSE_IOCTL_COMPAT set for 32bit ioctls in
 * 64bit environment.  The size and direction of data is
 * determined by _IOC_*() decoding of cmd.  For _IOC_NONE,
 * data will be NULL, for _IOC_WRITE data is out area, for
 * _IOC_READ in area and if both are set in/out area.  In all
 * non-NULL cases, the area is of _IOC_SIZE(cmd) bytes.
 *
 * However, in our case, this function only needs to print cache table to the log file .
 * 
 * Introduced in version 2.8
 */
int caching_ioctl(const char *, int cmd, void *arg,
                  struct fuse_file_info *, unsigned int flags, void *data)
{
    return 0;
}


// Initialise the operations. 
// You are not supposed to change this function.
void init_caching_oper()
{

    caching_oper.getattr = caching_getattr;
    caching_oper.access = caching_access;
    caching_oper.open = caching_open;
    caching_oper.read = caching_read;
    caching_oper.flush = caching_flush;
    caching_oper.release = caching_release;
    caching_oper.opendir = caching_opendir;
    caching_oper.readdir = caching_readdir;
    caching_oper.releasedir = caching_releasedir;
    caching_oper.rename = caching_rename;
    caching_oper.init = caching_init;
    caching_oper.destroy = caching_destroy;
    caching_oper.ioctl = caching_ioctl;
    caching_oper.fgetattr = caching_fgetattr;


    caching_oper.readlink = NULL;
    caching_oper.getdir = NULL;
    caching_oper.mknod = NULL;
    caching_oper.mkdir = NULL;
    caching_oper.unlink = NULL;
    caching_oper.rmdir = NULL;
    caching_oper.symlink = NULL;
    caching_oper.link = NULL;
    caching_oper.chmod = NULL;
    caching_oper.chown = NULL;
    caching_oper.truncate = NULL;
    caching_oper.utime = NULL;
    caching_oper.write = NULL;
    caching_oper.statfs = NULL;
    caching_oper.fsync = NULL;
    caching_oper.setxattr = NULL;
    caching_oper.getxattr = NULL;
    caching_oper.listxattr = NULL;
    caching_oper.removexattr = NULL;
    caching_oper.fsyncdir = NULL;
    caching_oper.create = NULL;
    caching_oper.ftruncate = NULL;
}

int isDir(char const *path)
{
    int isDir = 1;
    struct stat s;
    int statRet = stat(path, &s);
    if (statRet == FAILURE)
    {
        isDir = 0;
    }
    else if (!S_ISDIR(s.st_mode))
    {
        isDir = 0;
    }


    return isDir;
}

void invalidUsage()
{
    cout << USAGE << endl;
    exit(0);
}

PrivateData *initPrivateData(char *const *argv, long numOfBlocks, long blockSize)
{
    PrivateData *data = new PrivateData;
    data->_blockSize = blockSize;
    data->_numOfBlocks = numOfBlocks;

    data->_rootDir = realpath(argv[1], NULL);
    if (data->_rootDir == NULL)
    {
        handleSystemError(ROOTDIR_ERROR_MSG);
    }
    // rootDir is to long to create the logger file
    if (strlen(data->_rootDir) + strlen(LOGGER_FILENAME) >= PATH_MAX)
    {
        handleSystemError(ROOTDIR_TOO_LONG_ERROR_MSG);
    }

    char loggerPath[PATH_MAX];
    strcpy(loggerPath, data->_rootDir);
    strncat(loggerPath, LOGGER_FILENAME, strlen(LOGGER_FILENAME));

    data->_log = openLogger(loggerPath);
    if (data->_log == NULL)
    {
        handleSystemError(OPEN_LOGGER_ERROR_MSG);
    }

    data->_blocks = new(std::nothrow) CacheBlock[numOfBlocks];
    if (data->_blocks == NULL)
    {
        handleSystemError(CACHE_ALLOC_ERROR_MSG);
    }

    return data;
}

//basic main. You need to complete it.
int main(int argc, char *argv[])
{
    int validArgs = 1;
    long numOfBlocks, blockSize;

    if (argc != 5)
    {
        validArgs = 0;
    }
    else
    {
        validArgs &= isDir(argv[1]) & isDir(argv[2]);
        validArgs &= (((numOfBlocks = strtol(argv[3], NULL, 10)) > 0) && errno == 0 &&
                      numOfBlocks <= INT_MAX);
        validArgs &= (((blockSize = strtol(argv[4], NULL, 10)) > 0) && errno == 0 &&
                      blockSize <= INT_MAX);
    }

    if (!validArgs)
    {
        invalidUsage();
    }

    init_caching_oper();

    PrivateData *data = initPrivateData(argv, numOfBlocks, blockSize);

    argv[1] = argv[2];
    for (int i = 2; i < (argc - 1); i++)
    {
        argv[i] = NULL;
    }
    argv[2] = (char *) "-s";
    //TODO: remove
    argv[3] = (char *) "-f";
    argc = 4;

    int fuse_stat = fuse_main(argc, argv, &caching_oper, data);
    return fuse_stat;
}