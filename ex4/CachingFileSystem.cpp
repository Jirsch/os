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
#include <cstdio>
#include <cstdlib>
#include "Logger.h"
#include <algorithm>
#include <math.h>

using namespace std;

struct fuse_operations caching_oper;

/*
 * return the index of the least frequently used block
 */
int getLFU()
{
    // make the first block the default minimum
    int minAccesses = STATE->_blocks[0]._accessCounter;
    int LFUIndex = 0;

    // going through the rest of the blocks and finding the minimum accessed one
    for (int i = sizeof(CacheBlock); i < STATE->_numOfBlocks * sizeof(CacheBlock); i += sizeof(CacheBlock)) // todo: is this how we increment i?
    {
        if (STATE->_blocks[i]._accessCounter < minAccesses)
        {
            minAccesses = STATE->_blocks[i]._accessCounter;
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
int caching_getattr(const char *path, struct stat *statbuf){
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
int caching_fgetattr(const char *path, struct stat *statbuf, struct fuse_file_info *fi){
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
int caching_open(const char *path, struct fuse_file_info *fi){
    return 0;
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
        return min(reqEnd - blockStart, reqEnd - reqStart, blockEnd - blockStart);
    }

    // check if we need to read from the end of the block (or the whole block)
    else
    {
        return min(blockEnd - reqStart, reqEnd - reqStart, blockEnd - blockStart);
    }
}

/*
 * return the index in the cache that the new block should be inserted to
 */
int getIndexToInsert()
{
    // checking if the cache is full
    if (STATE->_numOfTakenBlocks < STATE->_numOfBlocks)
    {
        return (STATE->_numOfTakenBlocks)++;
    }

    // the cache is full, we'll find the least frequently used block index
    return getLFU();
}

/*
 * initialize a bool array that will hold a bool var for each byte in the buf that will be true
 * if it has been read, false o.w
 */
bool* initHasByteBeenRead(size_t size)
{
    bool hasByteBeenRead[size];

    // initializing each byte to false
    for (int i = 0; i < size; i++)
    {
        hasByteBeenRead[i] = false;
    }

    return hasByteBeenRead;
}

/*
 * reads data from a block. returns the number of bytes that was read
 */
size_t readFromBlock(CacheBlock* block, char* buf, size_t start, size_t end, size_t bufOffset, bool* hasByteBeenRead)
{
    size_t bytesToReadFromBlock = getNumOfBytes(block->_start, block->_end, start, end);

    // reading the requested bytes from the block to the buffer
    memcpy(buf + bufOffset, (block->_data) + getOffset(start, block->_start), bytesToReadFromBlock);

    (block->_accessCounter)++;

    // updating the read buf bytes
    for (int i = getOffset(block->_start, start); i < bytesToReadFromBlock; i++)
    {
        hasByteBeenRead[i] = true;
    }

    return bytesToReadFromBlock;
}

/*
 * return the byte that starts the block of the current byte
 */
off_t getStartOfBlock(size_t curByte, off_t offset)
{
    return floor((curByte + offset) / STATE->_blockSize) * STATE->_blockSize + 1; // todo: +1?
}

/*
 * go over the blocks in the cache and read data from the relevant blocks
 */
void readDataFromCache(const char *path, char *buf, size_t size, off_t offset, bool* hasByteBeenRead)
{
    size_t startOfReading = offset;
    size_t endOfReading = offset + size;

    for (int i = 0; i < STATE->_numOfBlocks; i++) // todo: is this how we increment i?
    {
        CacheBlock* cur = STATE->_blocks[i];

        // checking if there are no further blocks in the cache
        if (cur == NULL)
        {
            break;
        }

        // checking if the current block is part of the requested file
        if (strcmp(path, cur->_fileName) != 0)
        {
            // checking if we need to read the current block
            if (startOfReading >= cur->_start || endOfReading <= cur->_end)
            {
                readFromBlock(cur, buf, startOfReading, endOfReading, getOffset(cur->_start, startOfReading), hasByteBeenRead);
            }
        }
    }
}

/*
 * go over the buffer and read from the disc the data that wasn't in the cache
 */
void readDataFromDisc(const char *path, char *buf, size_t size, off_t offset, bool* hasByteBeenRead, struct fuse_file_info *fi)
{
    for (size_t curByte = 0; curByte < size; curByte++)
    {
        // checking if the current byte has been read already
        if (hasByteBeenRead[curByte] == false)
        {
            // finding the byte that starts the block of the current byte
            int startOfBlock = getStartOfBlock(curByte, offset);

            // initializing the data and reading it from the block
            char* retrievedData[STATE->_blockSize];
            pread(fi->fh, retrievedData, STATE->_blockSize, startOfBlock); // todo: do we need to delete retrievedData?

            // initializing a new block and reading from it to the buffer
            CacheBlock* newBlock = new CacheBlock(path, startOfBlock, startOfBlock + STATE->_blockSize, retrievedData); // todo: STATE->_blockSize -1?
            size_t bytesToReadFromBlock = readFromBlock(newBlock, buf, curByte + offset, offset + size, curByte, hasByteBeenRead);

            // finding the least frequently used block in the cache and deleting it
            int indexToInsert = getIndexToInsert();
            delete STATE->_blocks[indexToInsert];

            // adding the new block insted of the LFU one
            STATE->_blocks[indexToInsert] = newBlock;

            curByte += bytesToReadFromBlock;
        }
    }
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
                 struct fuse_file_info *fi){
    //todo: convert path
    // todo: log "UNIX_TIME read"

    // will hold a bool var for each byte in the buf - true if it has been read, false o.w
    bool* hasByteBeenRead = initHasByteBeenRead(size);

    // looking in the cache for blocks that hold requested data and copying it to the buffer
    readDataFromCache(path, buf, size, offset, hasByteBeenRead);

    // reading the rest of the requested data from the disk
    readDataFromDisc(path, buf, size, offset, hasByteBeenRead, fi);
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
int caching_release(const char *path, struct fuse_file_info *fi){
    return 0;
}

/** Open directory
 *
 * This method should check if the open operation is permitted for
 * this  directory
 *
 * Introduced in version 2.3
 */
int caching_opendir(const char *path, struct fuse_file_info *fi){
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
                    struct fuse_file_info *fi){
    return 0;
}

/** Release directory
 *
 * Introduced in version 2.3
 */
int caching_releasedir(const char *path, struct fuse_file_info *fi){
    return 0;
}

/** Rename a file */
int caching_rename(const char *path, const char *newpath){
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
void *caching_init(struct fuse_conn_info *conn){
    return NULL;
}


/**
 * Clean up filesystem
 *
 * Called on filesystem exit.
 *
 * Introduced in version 2.3
 */
void caching_destroy(void *userdata){
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
int caching_ioctl (const char *, int cmd, void *arg,
                   struct fuse_file_info *, unsigned int flags, void *data){
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

//basic main. You need to complete it.
int main(int argc, char* argv[]){

    init_caching_oper();

    //TODO: make sure arguments are valid (4 arguments, valid sizes)
    //TODO: create Private data
    //TODO: Allocate cache
    //TODO: init log

    argv[1] = argv[2];
    for (int i = 2; i< (argc - 1); i++){
        argv[i] = NULL;
    }
    argv[2] = (char*) "-s";
    argc = 3;

    // TODO: pass in privateData
    int fuse_stat = fuse_main(argc, argv, &caching_oper, NULL);
    return fuse_stat;
}