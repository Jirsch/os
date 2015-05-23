//
// Created by Jonathan Hirsch on 5/22/15.
//

#include "Logger.h"
#include "CacheState.h"


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
