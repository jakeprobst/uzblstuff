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
#include <set>
#include <list>
#include <pthread.h>
#include <basedir.h>

#include "context.h"
#include "cookierequest.h"
#include "cookie.h"

class CookieJar {
    private:
        xdgHandle xdg;
        
        Context* ctx;
    
        int cookiefd;
    
        fd_set readfd;
        
        typedef bool (*cookiesorter)(const Cookie&, const Cookie&);
        typedef std::set<Cookie, cookiesorter> cookieSet;
        cookieSet cookies;
        
        void AddToQueue(int, char**);
        void HandleCookie(CookieRequest*);
        
        bool IsInWhitelist(const Cookie&);
        void RemoveDuplicates();
        
        void LoadFile();
    
    public:
        CookieJar(Context*);
        ~CookieJar();
    
        int ReadWhitelist(std::string &, std::set<std::string> &);
        void WriteFile();
        void Run();
    
};





















#endif /* _COOKIEJAR_H_ */
