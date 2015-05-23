//
// Created by Jonathan Hirsch on 5/22/15.
//

#include <time.h>
#include "Logger.h"
#include "CacheState.h"
#include <osxfuse/fuse.h>


FILE *openLogger(const char *path)
{
    FILE* file;

    file = fopen(path , "a");
    if (file!=NULL)
    {
        setvbuf(file, NULL, _IOLBF, 0);
    }

    return file;
}

int logFunctionEntry(const char *funcName)
{
    return fprintf( STATE->_log, "%lu %s\n",(u_long)time(NULL), funcName);
}
