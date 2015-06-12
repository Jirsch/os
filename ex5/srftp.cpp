//
// Created by orenbm21 on 6/5/2015.
//

// todo: check if need to remove includes
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>

#define ARG_COUNT 3
#define PORT_ARG_IDX 1
#define MAX_FILE_SIZE_ARG_IDX 2
#define SUCCESS 0
#define MIN_PORT 1
#define MAX_PORT 65535
#define MIN_FILE_SIZE 0
#define MAX_PENDING 5

#define FAILURE 1
#define SUCCESS 0

#define USAGE "Usage: srftp server-port max-file-size"
#define ERR_FUNC "Error: function:"
#define ERRNO "errno:"
#define NEW_LINE ".\n"

static const char *const GETHOSTBYNAME_FUNC = "gethostbyname";
static const char *const SOCKET_FUNC = "socket";
static const char *const BIND_FUNC = "bind";
static const char *const READ_FUNC = "read";
static const char *const WRITE_FUNC = "write";
static const char *const ACCEPT_FUNC = "accept";


bool isValidPort(int port)
{
    return port >= MIN_PORT && port <= MAX_PORT;
}

bool isValidMaxFile(size_t maxFileSize)
{
    return maxFileSize >= MIN_FILE_SIZE;
}

void exitOnSysErr(const char *name, int err_no)
{
    std::cerr << ERR_FUNC << name << ERRNO << err_no << NEW_LINE << std::endl;
    exit(FAILURE);
}

void exitOnIncorrectUsage()
{
    std::cout << USAGE << std::endl;
    exit(SUCCESS);
}

void validateInput(int argc, char* argv[])
{
    if (argc != ARG_COUNT)
    {
        exitOnIncorrectUsage();
    }

    long int portInput = strtol(argv[PORT_ARG_IDX], NULL, 10);
    long int maxFileSizeInput = strtol(argv[MAX_FILE_SIZE_ARG_IDX], NULL, 10);

    if (!isValidPort(portInput) || !isValidMaxFile(maxFileSizeInput))
    {
        exitOnIncorrectUsage();
    }
}

/*
 * return the socket that will accept requests from clients
 */
int connectToSocket(long int port)
{
    // todo: change to HOST_NAME_MAX of limits. it says so in forum but does not work for some reason
    char hostname[NI_MAXHOST + 1];
    struct sockaddr_in sa;
    struct hostent *entry;
    int socket;

    memset(&sa, 0, sizeof(struct sockaddr_in));
    gethostname(hostname, NI_MAXHOST);
    if ((entry = gethostbyname(hostname)) == NULL)
    {
        exitOnSysErr(GETHOSTBYNAME_FUNC, h_errno);
    }

    sa.sin_family = entry->h_addrtype;
    memcpy((char *) &sa.sin_addr, entry->h_addr, entry->h_length);
    sa.sin_port = htons(port);

    if ((socket = socket(AF_INET, SOCK_STREAM, 0)) < SUCCESS)
    {
        exitOnSysErr(SOCKET_FUNC, errno);
    }

    if (bind(socket, (struct sockaddr*) &sa, sizeof(struct sockaddr_in)) < SUCCESS)
    {
        close(socket);
        exitOnSysErr(BIND_FUNC, errno);
    }

    listen(socket, MAX_PENDING);

    return socket;
}

/*
 * return a new socket to contact with a client
 */
int getNewClientSocket(int socket) {

    int newSocket;

    if ((newSocket =  accept(socket,NULL,NULL)) < SUCCESS)
    {
        exitOnSysErr(ACCEPT_FUNC, errno);
    }
       
    return newSocket;
}

/*
 * read data from a given client's socket
 */
char* readData(const char *data, size_t size, int socket)
{
    size_t remains = size;
    ssize_t rc;
    while (remains > 0)
    {
        // todo: is the conversion to void* ok?
        if ((rc = read(socket, (void *) (data + size - remains), remains)) < SUCCESS)
        {
            exitOnSysErr(READ_FUNC, errno);
        }

        remains -= rc;
    }

    return (char*) data;
}

/*
 * read a number from a given client's socket
 */
uint32_t readNumber(int socket)
{
    uint32_t origin;
    char *data = (char *) &origin;

    origin = (uint32_t) readData(data, sizeof(origin), socket);

    return ntohl(origin);
}

/*
 * send data to a given client's socket
 */
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

/*
 * send a number to a given client's socket
 */
void sendNumber(uint32_t num, int socket)
{
    uint32_t converted = htonl(num);
    char *data = (char *) &converted;

    sendData(data, sizeof(converted), socket);
}

/*
 * read a file and its details from a client
 */
void readFromClient(int socket)
{
    // receive the file name length from the client
    uint32_t nameLen = readNumber(socket);

    // receive the file name
    char fileName[nameLen + 1];
    fileName = readData(fileName, nameLen, socket);

    // receive the file size
    uint32_t fileSize = readNumber(socket);

    // receive the file
    char fileData[fileSize + 1];
    fileData = readData(fileData, fileSize, socket);

    int file;
    if ((write(file, fileData, fileSize)) < SUCCESS)
    {
        exitOnSysErr(WRITE_FUNC, errno);
    }
}


int main(int argc, char* argv[])
{
    validateInput(argc, argv);

    long int port = strtol(argv[PORT_ARG_IDX], NULL, 10);
    long int maxFileSize = strtol(argv[MAX_FILE_SIZE_ARG_IDX], NULL, 10);

    // get the socket that will accept requests from clients
    int acceptingSocket = connectToSocket(port);

    // todo: contact with more than one client
    int clientSocket = getNewClientSocket(acceptingSocket);

    // send the max-file-size to the client
    sendNumber(maxFileSize, clientSocket);

    // read the file's details and then the file itself
    readFromClient(clientSocket);
}