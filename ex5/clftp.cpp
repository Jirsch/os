//
// Created by orenbm21 on 6/5/2015.
//

#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iostream>

static const int SUCCESS = 0;
static const char *const GETHOSTBYNAME_FUNC = "gethostbyname";
static const char *const SOCKET_FUNC = "socket";
static const char *const CONNECT_FUNC = "connect";
using namespace std;

void exitOnSysErr(const char *name, int err_no)
{
    std::cerr << "Error: function:" << name << "errno:" << err_no << ".\n" << std::endl;
    exit(1);
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

int main(int argc, char *argv[])
{
    //todo: check input

    char *hostname;
    ushort portnum;
    int socket = connectToServer(hostname, portnum);


    //todo: get file size
    //todo: open file


    //todo transmit file in chunks

    return SUCCESS;
}