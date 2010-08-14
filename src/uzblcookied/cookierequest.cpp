#include <unistd.h>
#include <stdlib.h>

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
        if (c[4])
            data = strdup(c[4]);
        else
            data = (char*)malloc(1);
    }
}


CookieRequest::~CookieRequest()
{
    free(cmd);
    free(protocol);
    free(host);
    free(path);
    free(data);
    close(fd);
}


