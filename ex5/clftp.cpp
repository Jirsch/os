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
#include <sys/time.h>

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

#define USAGE "Usage: clftp server-port server-hostname file-to-transfer filename-in-server"
#define ERR_FUNC "Error: function:"
#define ERRNO " errno:"
#define NEW_LINE ".\n"
#define SUCCESS 0
#define FAILURE 1

using namespace std;

/*
 * prints an error message and exits with 1
 */
void exitOnSysErr(const char *name, int err_no)
{
    std::cerr << ERR_FUNC << name << ERRNO << err_no << NEW_LINE << std::endl;
    exit(FAILURE);
}

/*
 * prints an "incorrect usage" message and exits with 0
 */
void exitOnIncorrectUsage()
{
    std::cout << USAGE << std::endl;
    exit(SUCCESS);
}

/*
 * return the socket to the server
 */
int connectToServer(const char *hostname, ushort port)
{
    struct sockaddr_in sa;
    struct hostent *entry;
    int serverSocket;

    if ((entry = gethostbyname(hostname)) == NULL)
    {
        exitOnSysErr(GETHOSTBYNAME_FUNC, h_errno);
    }

    memset(&sa, 0, sizeof(sa));
    sa.sin_family = entry->h_addrtype;
    memcpy((char *) &sa.sin_addr, entry->h_addr, entry->h_length);
    sa.sin_port = htons(port);

    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) < SUCCESS)
    {
        exitOnSysErr(SOCKET_FUNC, errno);
    }

    if (connect(serverSocket, (struct sockaddr *) &sa, sizeof(sa)) < SUCCESS)
    {
        exitOnSysErr(CONNECT_FUNC, errno);
    }

    return serverSocket;
}

/*
 * send data via the given socket
 */
void sendData(const char *data, size_t size, int socket)
{
    size_t remains = size;
    ssize_t rc;
    
    // send all the data
    while (remains > 0)
    {
        if ((rc = write(socket, data + size - remains, remains)) < SUCCESS)
        {
            exitOnSysErr(WRITE_FUNC, errno);
        }

        remains -= rc;
    }
}

/*
 * send a number to the given socket
 */
void sendNumber(uint32_t num, int socket)
{
    uint32_t converted = htonl(num);
    char *data = (char *) &converted;

    sendData(data, sizeof(converted), socket);
}

/*
 * read a number from the given socket
 */
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

/*
 * validate the input of the user
 */
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

/*
 * send the details of the file via the given socket
 */
void sendMetadata(int socket, const char *pathOnServer, uint32_t fileSize)
{
	// send the name length
    uint32_t nameLen = (uint32_t) strlen(pathOnServer) + 1;
    sendNumber(nameLen, socket);
    
    // send the file name in the server
    sendData(pathOnServer, nameLen, socket);
    
    // send the file size
    sendNumber(fileSize, socket);
}

/*
 * send the given file via the given socket
 */
void sendFile(int socket, int file, blksize_t blockSize)
{
    char fileBuffer[blockSize];
    ssize_t bytesRead;

    int position = 0;

    // read the file block by block and send it (also block by block)
    while ((bytesRead = pread(file, fileBuffer, blockSize, position)) > 0)
    {
        sendData(fileBuffer, bytesRead, socket);
        position += bytesRead;
    }
    if (bytesRead < SUCCESS)
    {
        exitOnSysErr(PREAD_FUNC, errno);
    }
}

int main(int argc, char *argv[])
{
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
    	cout << "Transmission failed: too big file" << endl;
    	close(socket);
    	
    	return SUCCESS;
    }

    int file;
    if ((file = open(localPath, O_RDONLY)) < SUCCESS)
    {
        exitOnSysErr(OPEN_FUNC, errno);
    }

    sendMetadata(socket, pathOnServer, fileSize);

    sendFile(socket, file, blockSize);

    close(socket);

    return SUCCESS;
}
