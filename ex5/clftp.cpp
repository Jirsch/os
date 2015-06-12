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
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>

static const int SUCCESS = 0;
static const char *const GETHOSTBYNAME_FUNC = "gethostbyname";
static const char *const SOCKET_FUNC = "socket";
static const char *const CONNECT_FUNC = "connect";
static const char *const STAT_FUNC = "stat";
static const char *const OPEN_FUNC = "open";
static const char *const PREAD_FUNC = "pread";
static const char *const READ_FUNC = "read";
static const char *const WRITE_FUNC = "write";
static const int MAX_PORT = 65535;
static const int MIN_PORT = 1;
static const int ARG_COUNT = 5;
static const int PORT_ARG_IDX = 1;
static const int LOCAL_PATH_ARG_IDX = 3;
static const int HOSTNAME_ARG_IDX = 2;
static const int SERVER_PATH_ARG_IDX = 4;
using namespace std;

void exitOnSysErr(const char *name, int err_no)
{
    std::cerr << "Error: function:" << name << "errno:" << err_no << ".\n" << std::endl;
    exit(1);
}

void exitOnIncorrectUsage()
{
    std::cout << "Usage: clftp server-port server-hostname file-to-transfer "
            "filename-in-server" << std::endl;
    exit(0);
}

int connectToServer(const char *hostname, ushort port)
{
    struct sockaddr_in sa;
    struct hostent *entry;
    int socket;

    if ((entry = gethostbyname(hostname)) == NULL)
    {
        exitOnSysErr(GETHOSTBYNAME_FUNC, h_errno);
    }

    memset(&sa, 0, sizeof(sa));
    sa.sin_family = entry->h_addrtype;
    memcpy((char *) &sa.sin_addr, entry->h_addr, entry->h_length);
    sa.sin_port = htons(port);

    if ((socket = socket(AF_INET, SOCK_STREAM, 0)) < SUCCESS)
    {
        exitOnSysErr(SOCKET_FUNC, errno);
    }

    if (connect(socket, (struct sockaddr *) &sa, sizeof(sa)) < SUCCESS)
    {
        exitOnSysErr(CONNECT_FUNC, errno);
    }

    return socket;
}

void sendData(const char *data, size_t size, int socket)
{
    size_t remains = size;
    ssize_t rc;
    while (remains > 0)
    {
        if ((rc = write(socket, data + size - remains, remains)) < SUCCESS)
        {
            exitOnSysErr(WRITE_FUNC, errno);
        }
        remains -= rc;
    }
}

void sendNumber(uint32_t num, int socket)
{
    uint32_t converted = htonl(num);
    char *data = (char *) &converted;

    sendData(data, sizeof(converted), socket);
}

uint32_t readNumber(int socket)
{
    uint32_t origin;
    char *data = (char *) &origin;
    size_t remains = sizeof(origin);

    ssize_t rc;
    while (remains > 0)
    {
        if ((rc = read(socket, data + sizeof(origin) - remains, remains)) < SUCCESS)
        {
            exitOnSysErr(READ_FUNC, errno);
        }
        remains -= rc;
    }

    return ntohl(origin);
}

ushort validateInput(int argc, char *argv[], struct stat &statBuf)
{
    if (argc != ARG_COUNT)
    {
        exitOnIncorrectUsage();
    }

    long int portInput = strtol(argv[PORT_ARG_IDX], NULL, 10);
    char *localPath = argv[LOCAL_PATH_ARG_IDX];

    if (portInput < MIN_PORT || portInput > MAX_PORT)
    {
        exitOnIncorrectUsage();
    }

    if (stat(localPath, &statBuf) != SUCCESS)
    {
        if (errno == ENOENT)
        {
            exitOnIncorrectUsage();
        }

        exitOnSysErr(STAT_FUNC, errno);
    }

    if (S_ISDIR(statBuf.st_mode))
    {
        exitOnIncorrectUsage();
    }

    return (ushort) portInput;
}

void sendMetadata(int socket, const char *pathOnServer, uint32_t fileSize)
{
    uint32_t nameLen = (uint32_t) strlen(pathOnServer) + 1;
    sendNumber(nameLen, socket);
    sendData(pathOnServer, nameLen, socket);
    sendNumber(fileSize, socket);
}

void sendFile(int socket, int file, blksize_t blockSize)
{
    char fileBuffer[blockSize];
    ssize_t bytesRead;

    int position = 0;
    while ((bytesRead = pread(file, fileBuffer, blockSize, position)) > 0)
    {
        sendData(fileBuffer, bytesRead, socket);
    }

    if (bytesRead < SUCCESS)
    {
        exitOnSysErr(PREAD_FUNC, errno);
    }
}

int main(int argc, char *argv[])
{
    //todo: check input
    struct stat statBuf;
    ushort portNum = validateInput(argc, argv, statBuf);

    char *hostname = argv[HOSTNAME_ARG_IDX];
    char *localPath = argv[LOCAL_PATH_ARG_IDX];
    char *pathOnServer = argv[SERVER_PATH_ARG_IDX];

    uint32_t fileSize = (uint32_t) statBuf.st_size;
    blksize_t blockSize = statBuf.st_blksize;

    int socket = connectToServer(hostname, portNum);

    uint32_t maxFileSize = readNumber(socket);
    if (fileSize > maxFileSize)
    {
        exit(SUCCESS);
    }

    int file;
    if ((file = open(localPath, O_RDONLY)) < SUCCESS)
    {
        exitOnSysErr(OPEN_FUNC, errno);
    }

    sendMetadata(socket, pathOnServer, fileSize);


    sendFile(socket, file, blockSize);

    return SUCCESS;
}