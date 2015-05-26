//
// Created by Jonathan Hirsch on 5/23/15.
//

#ifndef EX4_CACHESTATE_H
#define EX4_CACHESTATE_H


#include <limits.h>
#include <stdio.h>
#include <string.h>

#ifndef PATH_MAX
#define PATH_MAX 4086
#endif

#define STATE ((PrivateData *) fuse_get_context()->private_data)

typedef struct CacheBlock
{
    char *_fileName;
    int _accessCounter;
    size_t _start;
    size_t _end;
    char *_data;

    // constructor
    CacheBlock(char *fileName, size_t start, size_t end, char *data) : _start(start), _end(end)
    {
        _fileName = new char[strlen(fileName) + 1];
        memcpy(_fileName, fileName, strlen(fileName) + 1);

        _data = new char[end - start];
        memcpy(_data, data, end -start);

        _accessCounter = 0;
    }

    ~CacheBlock()
    {
        delete[] _fileName;
        delete[] _data;
    }

} CacheBlock;

typedef struct PrivateData
{
    size_t _blockSize;
    int _numOfBlocks;
    int _numOfTakenBlocks;
    CacheBlock** _blocks;
    char *_rootDir;
    FILE *_log;
} PrivateData;

#endif //EX4_CACHESTATE_H
