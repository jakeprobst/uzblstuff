#ifndef _COOKIEJAR_H_
#define _COOKIEJAR_H_

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/select.h>
#include <list>
#include <pthread.h>
#include <basedir.h>

#include "cookierequest.h"
#include "cookie.h"

class CookieJar {
    private:
        xdgHandle xdg;
    
        int cookiefd;
    
        fd_set readfd;
        
        std::list<Cookie*> cookies;
        
        void AddToQueue(int, char**);
        void HandleCookie(CookieRequest*);
        
        void RemoveDuplicates();
        
        void LoadFile();
    
    public:
        CookieJar();
        ~CookieJar();
    
        void WriteFile();
        void Run();
    
};





















#endif /* _COOKIEJAR_H_ */
