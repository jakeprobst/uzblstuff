#include <unistd.h>

#include "cookierequest.h"
#include "util.h"


CookieRequest::CookieRequest(int cfd, char** c)
{
    fd = cfd;
    cmd = strdup(c[0]);
    protocol = strdup(c[1]);
    host = strdup(c[2]);
    path = strdup(c[3]);
    
    data = NULL;
    if (!strcmp(cmd, "PUT")) {
        data = strdup(c[4]);
    }
}


CookieRequest::~CookieRequest()
{
    delete[] cmd;
    delete[] protocol;
    delete[] host;
    delete[] path;
    close(fd);
}


