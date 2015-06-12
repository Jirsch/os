//
// Created by orenbm21 on 6/5/2015.
//

#include "srftp.h"
#include "clftp.h"

#define VALID_NUM_OF_ARGS 3
#define SUCCESS 0
#define FAILURE 1

bool isValidPort(int port)
{
    return port >= 1 && port <= 65535;
}

bool isValidMaxFile(size_t maxFileSize)
{
    return maxFileSize >= 0;
}

int main(int argc, char* argv[])
{
    int validArgs = SUCCESS;
    if (argc != VALID_NUM_OF_ARGS || !isValidPort(argv[1]) || !isValidMaxFile(argv[2]))
    {
        validArgs = FAILURE;
    }
}