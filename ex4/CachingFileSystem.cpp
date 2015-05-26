/*
 * CachingFileSystem.cpp
 *
 *  Created on: 15 April 2015
 *  Author: Netanel Zakay, HUJI, 67808  (Operating Systems 2014-2015).
 */

#define FUSE_USE_VERSION 26

#include <fuse.h>
#include "CacheState.h"
#include "Logger.h"
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <dirent.h>
#include <unistd.h>
#include <math.h>

using namespace std;

struct fuse_operations caching_oper;


static const int FAILURE = -1;
static const int SYS_ERROR = 1;
static const char *const LOGGER_FILENAME = "/.filesystem.log";
static const char *const SYSTEM_ERROR_PREFIX = "System Error: ";
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

static const char *const READDIR_FUNC = "readdir";

static const char *const OPENDIR_FUNC = "opendir";

static const char *const RELEASE_FUNC = "release";

static const char *const FLUSH_FUNC = "flush";

static const char *const OPEN_FUNC = "open";

static const char *const ACCESS_FUNC = "access";

static const char *const GETATTR_FUNC = "getattr";

static const char *const FGETATTR_FUNC = "fgetattr";

static const char *const RENAME_FUNC = "rename";

static const char *const IOCTL_FUNC = "ioctl";

static const char *const READ_FUNC = "read";

void handleSystemError(const char *msg)
{
    std::cerr << SYSTEM_ERROR_PREFIX << msg << std::endl;
    exit(SYS_ERROR);
}

/** Translate relative to actual path
 * Only copies PATH_MAX - strlen(STATE->_rootDir)-1 chars
 */
static void toActualPath(char fpath[PATH_MAX], const char *path)
{
    strcpy(fpath, STATE->_rootDir);
    strncat(fpath, path, PATH_MAX - strlen(STATE->_rootDir) - 1);
}

int functionEntry(const char *path, const char *funcName)
{
    if (logFunctionEntry(funcName) < SUCCESS)
    {
        return -errno;
    }

    if (strncmp(path + 1, LOGGER_FILENAME, strlen(LOGGER_FILENAME) + 1) == 0)
    {
        return -ENOENT;
    }

    return SUCCESS;
}

/*
 * return the index of the least frequently used block
 */
int getLFU()
{
    // make the first block the default minimum
    int minAccesses = STATE->_blocks[0]->_accessCounter;
    int LFUIndex = 0;

    // going through the rest of the blocks and finding the minimum accessed one
    for (int i = 0; i < STATE->_numOfTakenBlocks; i++)
    {
        if (STATE->_blocks[i]->_accessCounter < minAccesses)
        {
            minAccesses = STATE->_blocks[i]->_accessCounter;
            LFUIndex = i;
        }
    }

    return LFUIndex;
}

/** Get file attributes.
 *
 * Similar to stat().  The 'st_dev' and 'st_blksize' fields are
 * ignored.  The 'st_ino' field is ignored except if the 'use_ino'
 * mount option is given.
 */
int caching_getattr(const char *path, struct stat *statbuf)
{
    int ret;
    if ((ret = functionEntry(path, GETATTR_FUNC)) != SUCCESS)
    {
        return ret;
    }

    // file path too long
    if (strlen(path) > PATH_MAX - strlen(STATE->_rootDir) - 1)
    {
        return -EINVAL;
    }

    char actualPath[PATH_MAX];
    toActualPath(actualPath, path);

    if (lstat(actualPath, statbuf) == FAILURE)
    {
        return -errno;
    }

    return SUCCESS;
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
    int ret;
    if ((ret = functionEntry(path, FGETATTR_FUNC)) != SUCCESS)
    {
        return ret;
    }

    if (strncmp(path, "/", 2) == 0)
    {
        return caching_getattr(path, statbuf);
    }

    if (fstat(fi->fh, statbuf) == FAILURE)
    {
        return -errno;
    }

    return SUCCESS;
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
    int ret;
    if ((ret = functionEntry(path, ACCESS_FUNC)) != SUCCESS)
    {
        return ret;
    }

    // file path too long
    if (strlen(path) > PATH_MAX - strlen(STATE->_rootDir) - 1)
    {
        return -EINVAL;
    }

    char actualPath[PATH_MAX];

    toActualPath(actualPath, path);

    if (access(actualPath, mask) == FAILURE)
    {
        return -errno;
    }

    return SUCCESS;
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
    int ret;
    if ((ret = functionEntry(path, OPEN_FUNC)) != SUCCESS)
    {
        return ret;
    }

    // file path too long
    if (strlen(path) > PATH_MAX - strlen(STATE->_rootDir) - 1)
    {
        return -EINVAL;
    }

    if ((fi->flags & 3) != O_RDONLY)
    {
        return -EACCES;
    }

    int fd;
    char actualPath[PATH_MAX];

    toActualPath(actualPath, path);
    fd = open(actualPath, fi->flags);

    if (fd < SUCCESS)
    {
        return -errno;
    }

    fi->fh = fd;

    return SUCCESS;
}

/*
 * return the relevant offset for reading/writing
 */
size_t getOffset(size_t firstStart, size_t secondStart)
{
    if (firstStart > secondStart)
    {
        return firstStart - secondStart;
    }

    else
    {
        return 0;
    }
}

/*
 * return the number of bytes that should be read
 */
size_t getNumOfBytes(size_t blockStart, size_t blockEnd, size_t reqStart, size_t reqEnd)
{
    // check if we need to read from the start of the block (or the whole block)
    if (blockStart >= reqStart)
    {
        return (reqEnd - blockStart < reqEnd - reqStart) ?
               min(reqEnd - blockStart, blockEnd - blockStart) :
               min(reqEnd - reqStart, blockEnd - blockStart);
    }

    // we need to read from the end of the block (or the whole block)
    return (blockEnd - reqStart < reqEnd - reqStart) ?
           min(blockEnd - reqStart, blockEnd - blockStart) :
           min(reqEnd - reqStart, blockEnd - blockStart);
}

/*
 * return the index in the cache that the new block should be inserted to
 */
int getIndexToInsert()
{
    // checking if the cache is not full
    if (STATE->_numOfTakenBlocks < STATE->_numOfBlocks)
    {
        STATE->_blocks[STATE->_numOfTakenBlocks] = NULL;
        return (STATE->_numOfTakenBlocks)++;
    }

    // the cache is full, we'll find the least frequently used block index
    return getLFU();
}

/*
 * initialize a bool array that will hold a bool var for each byte in the buf that will be true
 * if it has been read, false o.w
 */
void initHasBlockBeenRead(int numOfBlocks, bool *hasBlocksBeenRead)
{
    // initializing each block to its first byte
    for (int i = 0; i < numOfBlocks; i++)
    {
        hasBlocksBeenRead[i] = false;
    }
}

/*
 * reads data from a block. returns the number of bytes that were read
 */
size_t readFromBlock(CacheBlock *block, char *buf, size_t start, size_t end, size_t bufOffset)
{
    size_t bytesToReadFromBlock = getNumOfBytes(block->_start, block->_end, start, end);

    // reading the requested bytes from the block to the buffer
    memcpy(buf + bufOffset, (block->_data) + getOffset(start, block->_start), bytesToReadFromBlock);

    (block->_accessCounter)++;

    return bytesToReadFromBlock;
}

/*
 * return the byte that starts the block of the current byte
 */
off_t getStartOfBlock(size_t curByte)
{
    return (curByte / STATE->_blockSize) * STATE->_blockSize; // todo: +1?
}

/*
 * go over the blocks in the cache and read data from the relevant blocks
 * return the number f blocks read from cache
 */
int readDataFromCache(const char *path, char *buf, size_t size, off_t offset,
                      bool *hasBlockBeenRead)
{
    size_t startOfReading = offset;
    size_t endOfReading = offset + size;

    int bytesRead = 0;

    for (int i = 0; i < STATE->_numOfTakenBlocks; i++)
    {
        CacheBlock *cur = STATE->_blocks[i];

        // checking if the current block is part of the requested file
        if (strncmp(path, cur->_fileName, PATH_MAX) == 0)
        {
            // checking if we need to read the current block
            if (endOfReading >= cur->_start && startOfReading <= cur->_end)
            {
                bytesRead += readFromBlock(cur, buf, startOfReading, endOfReading,
                                           getOffset(cur->_start, startOfReading));

                int blockNum =
                        (cur->_end - startOfReading) / STATE->_blockSize;
                hasBlockBeenRead[blockNum] = true;
            }
        }
    }

    return bytesRead;
}

/*
 * go over the buffer and read from the disc the data that wasn't in the cache
 * return the number of bytes read
 */
int readDataFromDisc(const char *path, char *buf, int numOfBlocks, size_t size, off_t offset,
                     bool *hasBlockBeenRead, struct fuse_file_info *fi)
{
    int bytesRead;
    int bytesReadFromDisc = 0;

    for (int curBlock = 0; curBlock < numOfBlocks; curBlock++)
    {

        bytesRead = 0;

        // checking if the current byte has been read already
        if (!hasBlockBeenRead[curBlock])
        {
            // finding the byte that starts the block of the current byte
            int startOfBlock = getStartOfBlock(curBlock * STATE->_blockSize + offset);

            // initializing the data and reading it from the block
            char retrievedData[STATE->_blockSize];
            if ((bytesRead = pread(fi->fh, retrievedData, STATE->_blockSize, startOfBlock)) <
                SUCCESS)
            {
                return -errno;
            }

            // initializing a new block and reading from it to the buffer
            CacheBlock *newBlock = new CacheBlock(path, startOfBlock, startOfBlock + bytesRead-1,
                                                  retrievedData);
            bytesReadFromDisc += readFromBlock(newBlock, buf, offset, offset + size,
                                               getOffset(newBlock->_start, offset));

            // updating that the block was read
            hasBlockBeenRead[curBlock] = true;

            // finding the least frequently used block in the cache and deleting it
            int indexToInsert = getIndexToInsert();
            delete STATE->_blocks[indexToInsert];

            // adding the new block instead of the LFU one
            STATE->_blocks[indexToInsert] = newBlock;

            // todo: remove
            std::cout << "Disc block #: " << curBlock << " startOfBlock: " << startOfBlock <<
            " bytesRead: " << bytesRead << " cacheIdx: " << indexToInsert << std::endl;
        }
    }

    return bytesReadFromDisc;
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
    //todo: add treatment of too much reading

    //todo: remove
    std::cout << "path: " << path << " offset: " << offset << " size: " << size << std::endl;

    int ret;
    if ((ret = functionEntry(path, READ_FUNC)) != SUCCESS)
    {
        return ret;
    }

    // calculating the number of blocks that will be read
    int numOfBlocks = size % STATE->_blockSize != 0 ? size / STATE->_blockSize + 1 :
                      size / STATE->_blockSize;

    // will hold a bool var for each block in the buf - true if it has been read, false o.w
    bool hasBlockBeenRead[numOfBlocks];
    initHasBlockBeenRead(numOfBlocks, hasBlockBeenRead);

    // looking in the cache for blocks that hold requested data and copying it to the buffer
    size_t bytesRead = readDataFromCache(path, buf, size, offset, hasBlockBeenRead);

    // todo: remove
    std::cout << "bytesRead: " << bytesRead << std::endl;

    int blocksRemaining = 0;
    for (int i=0; i<numOfBlocks;++i)
    {
        blocksRemaining += hasBlockBeenRead[i] ? 0 : 1;
        // todo: remove
        std::cout << "block#: " << i << " read: " << hasBlockBeenRead[i] << std::endl;
    }

    // checking if we need to read from disc
    if (blocksRemaining > 0)
    {
        // reading the rest of the requested data from the disk
        if ((ret = readDataFromDisc(path, buf, numOfBlocks, size, offset, hasBlockBeenRead, fi)) <
            SUCCESS)
        {
            return -errno;
        }
        bytesRead += ret;
    }

    // todo: remove
    std::cout << "bytesRead: " << bytesRead << std::endl;

    return bytesRead;
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
    int ret;
    if ((ret = functionEntry(path, FLUSH_FUNC)) != SUCCESS)
    {
        return ret;
    }

    return SUCCESS;
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
    int ret;
    if ((ret = functionEntry(path, RELEASE_FUNC)) != SUCCESS)
    {
        return ret;
    }

    if (close(fi->fh) == FAILURE)
    {
        return -errno;
    }

    return SUCCESS;
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
    int ret;
    if ((ret = functionEntry(path, OPENDIR_FUNC)) != SUCCESS)
    {
        return ret;
    }

    // file path too long
    if (strlen(path) > PATH_MAX - strlen(STATE->_rootDir) - 1)
    {
        return -EINVAL;
    }

    char actualPath[PATH_MAX];
    toActualPath(actualPath, path);

    DIR *dir = opendir(actualPath);
    if (dir == NULL)
    {
        return -errno;
    }

    fi->fh = (intptr_t) dir;

    return SUCCESS;
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
    int ret;
    if ((ret = functionEntry(path, READDIR_FUNC)) != SUCCESS)
    {
        return ret;
    }

    //todo: ignore .filesystem.log
    DIR *dir = (DIR *) (uintptr_t) fi->fh;
    struct dirent *dEntry;

    // reset errno before calling readdir
    errno = SUCCESS;
    while ((dEntry = readdir(dir)) != NULL)
    {
        if (filler(buf, dEntry->d_name, NULL, 0) != SUCCESS)
        {
            return -EINVAL;
        }

        errno = SUCCESS;
    }

    if (errno != SUCCESS)
    {
        return -errno;
    }

    return SUCCESS;
}

/** Release directory
 *
 * Introduced in version 2.3
 */
int caching_releasedir(const char *path, struct fuse_file_info *fi)
{
    int ret;
    if ((ret = functionEntry(path, RELEASEDIR_FUNC)) != SUCCESS)
    {
        return ret;
    }

    if (closedir((DIR *) (uintptr_t) fi->fh) == FAILURE)
    {
        return -errno;
    }

    return SUCCESS;
}

/** Rename a file */
int caching_rename(const char *path, const char *newpath)
{
    int ret;
    if ((ret = functionEntry(path, RENAME_FUNC)) != SUCCESS)
    {
        return ret;
    }

    // file path too long
    if (strlen(path) > PATH_MAX - strlen(STATE->_rootDir) - 1 ||
        strlen(newpath) > PATH_MAX - strlen(STATE->_rootDir) - 1)
    {
        return -EINVAL;
    }

    char actualPath[PATH_MAX];
    char actualNewPath[PATH_MAX];

    toActualPath(actualPath, path);
    toActualPath(actualNewPath, newpath);

    if (rename(actualPath, actualNewPath) != SUCCESS)
    {
        return -errno;
    }

    // rename paths in cache
    for (int i = 0; i < STATE->_numOfTakenBlocks; ++i)
    {
        if (strncmp(path, STATE->_blocks[i]->_fileName, PATH_MAX) == 0)
        {
            delete[] STATE->_blocks[i]->_fileName;

            STATE->_blocks[i]->_fileName = new char[strlen(newpath) + 1];
            memcpy(STATE->_blocks[i]->_fileName, newpath, strlen(newpath) + 1);
        }
    }

    return SUCCESS;
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
    closeLogger(STATE->_log);
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
    if (logFunctionEntry(IOCTL_FUNC) < SUCCESS)
    {
        return -errno;
    }

    for (int i = 0; i < STATE->_numOfTakenBlocks; ++i)
    {
        if (logCacheBlock(STATE->_blocks[i]) < SUCCESS)
        {
            return -errno;
        }
    }

    return SUCCESS;
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

    data->_blocks = new(std::nothrow) CacheBlock *[numOfBlocks];
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