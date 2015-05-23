//
// Created by Jonathan Hirsch on 5/23/15.
//

#ifndef EX4_CACHESTATE_H
#define EX4_CACHESTATE_H


#include <limits.h>
#include <stdio.h>

#ifndef PATH_MAX
#define PATH_MAX 4086
#endif

typedef struct CacheBlock{
    char* _fileName;
    int _accessCounter;
    size_t _start;
    size_t _end;
    char* _data;
} CacheBlock;

typedef struct PrivateData{
    size_t _blockSize;
    int _numOfBlocks;
    CacheBlock* _blocks;
    char* _rootDir;
    FILE* _log;
} PrivateData;

#define STATE ((PrivateData *) fuse_get_context()->private_data)
#endif //EX4_CACHESTATE_H
