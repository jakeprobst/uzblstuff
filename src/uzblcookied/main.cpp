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

Context *ctx;

void sigtermhandle(int a)
{
    ctx->running = false;
    ctx->log(1, "Singal caught and handled.");
}

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
    
    flock l;
    memset(&l, 0, sizeof(l));
    l.l_type = F_WRLCK;
    l.l_whence = SEEK_SET;

    if (fcntl(fd, F_SETLK, &l) == -1) {
        std::cerr<<"Can't obtain lock. Probably uzblcookied is already started."<<std::endl;
        exit(1);
    }

    /* Detach from terminal */
    if (ctx->daemonize) {
        close(0);
        close(1);
        close(2);
    }

    /* Write pid to pidfile */
    char pid[16];
    memset(pid, 0, 16);
    sprintf(pid, "%i\n", getpid());
    write(fd, pid, strlen(pid));
    fsync(fd);

    /* Register signal handler */
    struct sigaction sigact;
    sigact.sa_handler=sigtermhandle;
    sigemptyset (&sigact.sa_mask);
    sigact.sa_flags = 0;
    if (sigaction(SIGINT, &sigact, NULL)) ctx->perror("sigaction");
    if (sigaction(SIGTERM, &sigact, NULL)) ctx->perror("sigaction");
    
    CookieJar* cookiejar = new CookieJar(ctx);
    
    cookiejar->Run();
    if (!ctx->nowrite) {
        ctx->log(1, "Trying to write cookies file.");
        cookiejar->WriteFile();
    }

    delete cookiejar;
    close(fd);
    if (unlink(pidfile)) ctx->perror("unlink");

    delete ctx;
    
    return 1;
}
