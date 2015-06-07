//
// Created by Jonathan Hirsch on 5/22/15.
//

#ifndef EX4_LOGGER_H
#define EX4_LOGGER_H

#include <stdio.h>
#include "CacheState.h"

FILE* openLogger(const char* path);
int logFunctionEntry(const char* funcName);
int logCacheBlock(const CacheBlock* block);
void closeLogger(FILE* logger);

#endif //EX4_LOGGER_H
