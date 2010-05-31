#include "cookiejar.h"
#include <signal.h>
#include <sys/stat.h>

void sigtermhandle(int a)
{
    throw 12; // random number, throw something
}

int main()
{
    
    int fd = open("/tmp/uzblcookied", O_RDWR|O_CREAT);
    fchmod(fd, 0666);
    
    flock* l = new flock;
    l->l_type = F_WRLCK;
    l->l_whence = SEEK_SET;
    
    if (fcntl(fd, F_SETLK, l) == -1)
        return 1;
    
    signal(SIGTERM, &sigtermhandle);
    signal(SIGINT, &sigtermhandle);
    
    CookieJar* cookiejar = new CookieJar();
    
    try {
        cookiejar->Run();
    }
    catch (int e) {
        cookiejar->WriteFile();
    }
    delete cookiejar;
    
    return 1;
}
