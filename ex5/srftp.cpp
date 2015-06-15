//
// Created by orenbm21 on 6/5/2015.
//

#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>
#include <fstream>
#include <limits.h>
#include <algorithm>

#define SUCCESS 0
#define FAILURE 1
#define ARG_COUNT 3
#define PORT_ARG_IDX 1
#define MAX_FILE_SIZE_ARG_IDX 2
#define MIN_PORT 1
#define MAX_PORT 65535
#define MIN_FILE_SIZE 0
#define MAX_PENDING 5
#define BLOCK_SIZE 4096
#define USAGE "Usage: srftp server-port max-file-size"
#define ERR_FUNC "Error: function:"
#define ERRNO " errno:"
#define NEW_LINE ".\n"

static const char *const PTHREAD_CREATE_FUNC = "pthread_create";
static const char *const OFSTREAM_WRITE_FUNC = "ofstream write";
static const char *const GETHOSTBYNAME_FUNC = "gethostbyname";
static const char *const SOCKET_FUNC = "socket";
static const char *const BIND_FUNC = "bind";
static const char *const READ_FUNC = "read";
static const char *const WRITE_FUNC = "write";
static const char *const ACCEPT_FUNC = "accept";

using namespace std;

/*
 * a struct that holds the socket and max-file-size to transfer
 */
typedef struct SocketData
{

private:
    int* _clientSocket;
    long int _maxFileSize;

public:

    /*
     * constructor
     */
    SocketData(int clientSocket, long int maxFileSize) : _maxFileSize(maxFileSize)
    {
        _clientSocket = new int(clientSocket);
    }

    /*
     * return the socket
     */
    int* getSocket()
    {
        return _clientSocket;
    };

    /*
     * return the max-file-size to transfer
     */
    long int getMaxFileSize()
    {
        return _maxFileSize;
    }

    /*
     * destructor
     */
    ~SocketData()
    {
        delete _clientSocket;
        _clientSocket = NULL;
    }

} SocketData;

/*
 * return true if the port is valid (between 1 and 65535)
 */
bool isValidPort(int port)
{
    return port >= MIN_PORT && port <= MAX_PORT;
}

/*
 * return true if the max-file-size is valid (higher than zero)
 */
bool isValidMaxFile(size_t maxFileSize)
{
    return maxFileSize >= MIN_FILE_SIZE;
}

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
 * validates that the user input is valid
 */
void validateInput(int argc, char* argv[])
{
    // check that the input has the right amount of arguments
    if (argc != ARG_COUNT)
    {
        exitOnIncorrectUsage();
    }

    // get the port and max-file-size inputs
    long int portInput = strtol(argv[PORT_ARG_IDX], NULL, 10);
    long int maxFileSizeInput = strtol(argv[MAX_FILE_SIZE_ARG_IDX], NULL, 10);

    // validate the port and max-file-size
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
    char hostname[NAME_MAX + 1];
    struct sockaddr_in sa;
    struct hostent *entry;
    int clientSocket;

    memset(&sa, 0, sizeof(struct sockaddr_in));
    gethostname(hostname, NI_MAXHOST);
    if ((entry = gethostbyname(hostname)) == NULL)
    {
        exitOnSysErr(GETHOSTBYNAME_FUNC, h_errno);
    }

    sa.sin_family = entry->h_addrtype;
    memcpy((char *) &sa.sin_addr, entry->h_addr, entry->h_length);
    sa.sin_port = htons(port);

    if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) < SUCCESS)
    {
        exitOnSysErr(SOCKET_FUNC, errno);
    }

    if (bind(clientSocket, (struct sockaddr*) &sa, sizeof(struct sockaddr_in)) < SUCCESS)
    {
        close(clientSocket);
        exitOnSysErr(BIND_FUNC, errno);
    }

    listen(clientSocket, MAX_PENDING);

    return clientSocket;
}

/*
 * return a new socket to contact with a client
 */
int getNewClientSocket(int socket) {

    int newSocket;

    // wait for a new connection
    if ((newSocket =  accept(socket,NULL,NULL)) < SUCCESS)
    {
        exitOnSysErr(ACCEPT_FUNC, errno);
    }

    return newSocket;
}

/*
 * read a file from a given client's socket and write it to working directory
 */
void copyFileFromSocket(char *fileName, size_t size, int socket)
{
    size_t remains = size;
    ssize_t rc;

    // initializing an ofstream object with the given file name
    ofstream file(fileName);

    // allocate a buffer in BLOCK_SIZE size
    char buf[BLOCK_SIZE];

    // read content of file block by block
    while (remains > 0)
    {
        // read BLOCK_SIZE bytes from the stream
        if ((rc = read(socket, buf, BLOCK_SIZE)) < SUCCESS)
        {
            exitOnSysErr(READ_FUNC, errno);
        }

        // write it to the file
        if ((file.write(buf, rc)) < SUCCESS)
        {
            exitOnSysErr(OFSTREAM_WRITE_FUNC, errno);
        }

        // update the number of bytes remaining
        remains -= rc;
    }
    
    file.close();
}

/*
 * read data of size size from a given client's socket into buf
 */
void readData(char* buf, size_t size, int socket)
{
    size_t remains = size;
    ssize_t rc;

    // reading the data from the socket into buf
    while (remains > 0)
    {
        if ((rc = read(socket, buf + size - remains, remains)) < SUCCESS)
        {
            exitOnSysErr(READ_FUNC, errno);
        }
        remains -= rc;
    }
}

/*
 * read a number from a given client's socket
 */
uint32_t readNumber(int socket)
{
    uint32_t origin;
    char *data = (char *) &origin;

    readData(data, sizeof(origin), socket);

    return ntohl(origin);
}

/*
 * send data to a given client's socket
 */
void sendData(const char *data, size_t size, int socket)
{
    size_t remains = size;
    ssize_t rc;

    // write content to socket
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
    char fileName[nameLen];
    readData(fileName, nameLen, socket);

    // receive the file size
    uint32_t fileSize = readNumber(socket);

    // receive the file and write it to working directory
    copyFileFromSocket(fileName, fileSize, socket);
}

/*
 * contacting with a client through a given socket.
 * first send the max-file-size and if the client sends a file, read it and save to working directory
 */
void* contactWithClient(void* args)
{
    SocketData *socketData = (SocketData*) args;

    // send the max-file-size to the client
    sendNumber(socketData->getMaxFileSize(), *(socketData->getSocket()));

    // read the file's details and then the file itself
    readFromClient(*(socketData->getSocket()));

    delete socketData;
    pthread_exit(NULL);
}

/*
 * create a new thread for a given socket which will run the given function
 */
void createSocketThread(int socket, long int maxFileSize, void* f (void*))
{
    SocketData *args = new SocketData(socket, maxFileSize);

    // open thread for connecting with client
    pthread_t socketThread;
    if ((pthread_create(&socketThread, NULL, f, args)) < SUCCESS)
    {
        exitOnSysErr(PTHREAD_CREATE_FUNC, errno);
    }
}

/*
 * get new client connections while clients are connenting to the port
 */
void acceptConnections(int acceptingSocket, long int maxFileSize)
{
    // start contacting with clients
    while (true)
    {
        int clientSocket = getNewClientSocket(acceptingSocket);

        // creating a thread for the new socket
        createSocketThread(clientSocket, maxFileSize, &contactWithClient);
    }
}

int main(int argc, char* argv[])
{
    validateInput(argc, argv);

    long int port = strtol(argv[PORT_ARG_IDX], NULL, 10);
    long int maxFileSize = strtol(argv[MAX_FILE_SIZE_ARG_IDX], NULL, 10);

    // get the socket that will accept requests from clients
    int acceptingSocket = connectToSocket(port);

    // accept new connections
    acceptConnections(acceptingSocket, maxFileSize);
}
