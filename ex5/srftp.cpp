//
// Created by orenbm21 on 6/5/2015.
//


#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iostream>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <unistd.h>

#define ARG_COUNT 3
#define PORT_ARG_IDX 1
#define MAX_FILE_SIZE_ARG_IDX 2
#define SUCCESS 0
#define FAILURE 1

#define MIN_PORT 1

#define MAX_PORT 65535

#define MIN_FILE_SIZE 0

bool isValidPort(int port)
{
    return port >= MIN_FILE_SIZE && port <= MAX_PORT;
}

bool isValidMaxFile(size_t maxFileSize)
{
    return maxFileSize >= MIN_FILE_SIZE;
}

void exitOnSysErr(const char *name, int err_no)
{
    std::cerr << "Error: function:" << name << "errno:" << err_no << ".\n" << std::endl;
    exit(1);
}

void exitOnIncorrectUsage()
{
    std::cout << "Usage: srftp server-port max-file-size" << std::endl;
    exit(0);
}

ushort validateInput(int argc, char* argv[])
{
    if (argc != ARG_COUNT)
    {
        exitOnIncorrectUsage();
    }

    long int portInput = strtol(argv[PORT_ARG_IDX], NULL, 10);
    long int maxFileSize = argv[MAX_FILE_SIZE_ARG_IDX];

    if (!isValidPort(argv[PORT_ARG_IDX]) || !!isValidMaxFile(argv[MAX_FILE_SIZE_ARG_IDX]))
    {
        exitOnIncorrectUsage();
    }
    return (ushort) portInput;
}


int main(int argc, char* argv[])
{
    ushort input = validateInput(argc, argv);

    long int portInput = strtol(argv[PORT_ARG_IDX], NULL, 10);
    long int maxFileSize = strtol(argv[MAX_FILE_SIZE_ARG_IDX], NULL, 10);




}