#include "cookiejar.h"
#include "context.h"
#include <signal.h>
#include <sys/stat.h>
#include <basedir.h>
#include <unistd.h>
#include <cstring>

void help() {
    printf("Usage: uzblcookied [-vfmn]\n"
            "    -v - be verbose (use multiple times to increase verbosity level)\n"
            "    -f - foreground (do not detach from terminal)\n"
            "    -m - operate in memory (write cookies only at exit)\n"
            "    -n - do not write to cookies.txt at all\n");
}

void sigtermhandle(int a)
{
    throw 12; // random number, throw something
}

Context *ctx;

int main(int argc, char **argv)
{
    ctx = new Context(argc, argv);

    if (ctx->help) {
        help();
        exit(0);
    }

    /* Daemonize */
    if (ctx->daemonize) {
        switch (fork()) {
            case 0:
                /* we are the daemon process (Har! Har!) */
                close(0);
                close(1);
                close(2);
                break;
            case -1:
                perror("fork");
                exit(1);
            default:
                exit(0);
        }
    }

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
    
    /* Register signal handler */
    struct sigaction sigact;
    sigact.sa_handler=&sigtermhandle;
    sigemptyset (&sigact.sa_mask);
    sigact.sa_flags = 0;
    if (sigaction(SIGINT, &sigact, NULL)) ctx->perror("sigaction");
    if (sigaction(SIGTERM, &sigact, NULL)) ctx->perror("sigaction");
    
    CookieJar* cookiejar = new CookieJar(ctx);
    
    try {
        cookiejar->Run();
    }
    catch (int e) {
        if (!ctx->nowrite) {
            cookiejar->WriteFile();
            ctx->log(1, "Cookies file wrote.");
        }
    }
    delete cookiejar;
    close(fd);
    if (unlink(pidfile)) ctx->perror("unlink");
    
    return 1;
}
