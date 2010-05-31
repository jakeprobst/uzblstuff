#include "cookiejar.h"
#include <signal.h>
#include <sys/stat.h>
#include <basedir.h>
#include <unistd.h>
#include <cstring>

void sigtermhandle(int a)
{
    throw 12; // random number, throw something
}

int main()
{
    
    /* Create pidfile and use it as lock */
    xdgHandle xdg;
    xdgInitHandle(&xdg);
    char pidfile[1024*4];
    memset(pidfile, 0, 1024*4);
    sprintf(pidfile, "%s/uzbl/uzblcookied", xdgCacheHome(&xdg));
    xdgWipeHandle(&xdg);
    int fd = open(pidfile, O_RDWR|O_CREAT, 0600);
    
    flock* l = new flock;
    l->l_type = F_WRLCK;
    l->l_whence = SEEK_SET;
    
    char pid[16];
    memset(pid, 0, 16);
    sprintf(pid, "%i\n", getpid());
    write(fd, pid, strlen(pid));
    fsync(fd);

    if (fcntl(fd, F_SETLK, l) == -1)
        return 1;
    
    /* Registeer signal handler */
    struct sigaction sigact;
    sigact.sa_handler=&sigtermhandle;
    sigemptyset (&sigact.sa_mask);
    sigact.sa_flags = 0;
    if (sigaction(SIGINT, &sigact, NULL)) perror("sigaction");
    if (sigaction(SIGTERM, &sigact, NULL)) perror("sigaction");
    
    CookieJar* cookiejar = new CookieJar();
    
    try {
        cookiejar->Run();
    }
    catch (int e) {
        cookiejar->WriteFile();
    }
    delete cookiejar;
    close(fd);
    if (unlink(pidfile)) perror ("unlink");
    
    return 1;
}
