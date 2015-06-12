//
// Created by Jonathan Hirsch on 5/22/15.
//

#include <time.h>
#include "Logger.h"
#include "CacheState.h"
#include <fuse.h>


FILE *openLogger(const char *path)
{
    FILE *file;

    file = fopen(path, "a");
    if (file != NULL)
    {
        setvbuf(file, NULL, _IOLBF, 0);
    }

    return file;
}

int logFunctionEntry(const char *funcName)
{
    return fprintf(STATE->_log, "%lu %s\n", (u_long) time(NULL), funcName);
}

int logCacheBlock(const CacheBlock *block)
{
    return fprintf(STATE->_log, "%s %d %d\n", block->_fileName,
                   (int) (block->_start / STATE->_blockSize) + 1, block->_accessCounter);
}


void closeLogger(FILE *logger)
{
    fclose(logger);
}
