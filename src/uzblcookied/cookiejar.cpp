#include "cookiejar.h"
#include "util.h"

const char* SOCKETFILE = "uzbl/cookie_daemon_socket";

bool domain_match(char* a, char* b)
{
    if (startswith(b, ".") && endswith(a, b))
        return true;
    if (!startswith(b, ".") && !strcmp(a, b))
        return true;
    return false;
}


CookieJar::CookieJar()
{
    cookiefd = socket(AF_UNIX, SOCK_SEQPACKET, 0);
    //printf("socket: %s\n", strerror(errno));

    sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    sprintf(addr.sun_path, "%s/%s", getenv("XDG_CACHE_HOME"), SOCKETFILE);
    
    unlink(addr.sun_path);
    //printf("path: %s\n", addr.sun_path);
    
    bind(cookiefd, (sockaddr*)&addr, sizeof(addr));
    //printf("bind: %s\n", strerror(errno));
    
    listen(cookiefd, 10);
    //printf("listen: %s\n", strerror(errno));

    LoadFile();
}

CookieJar::~CookieJar()
{
    close(cookiefd);
}

void CookieJar::LoadFile()
{
    char buf[1024*4];
    memset(buf, 0, 1024*4);
    sprintf(buf, "%s/uzbl/cookies.txt", getenv("XDG_DATA_HOME"));
    
    FILE* f = fopen(buf, "r");
    if (f == NULL)
        return;
    memset(buf, 0, 1024*4);
    
    int c, n = 0;
    while (n != 2) {
        c = fgetc(f);
        if (c == EOF)
            return;
        if (c == '\n')
            n++;
        else
            n = 0;
    }
    
    int b = 0;
    while (true) {
        c = fgetc(f);
        if (c == EOF)
            break;
        if (c == '\n') {
            Cookie* c = new Cookie(buf);
            cookies.push_back(c);
            
            memset(buf, 0, b);
            b = 0;
            continue;
        }
        
        buf[b++] = c;
    }
}

void CookieJar::WriteFile()
{
    cookies.sort(cookiesort);
    
    char buf[1024*4];
    memset(buf, 0, 1024*4);
    
    std::list<char*>::iterator witer;
    std::list<char*> whitelist;
    sprintf(buf, "%s/uzbl/cookie_whitelist", getenv("XDG_DATA_HOME"));
    FILE* f = fopen(buf, "r");
    if (f != NULL) {
        memset(buf, 0, 1024*4);
        int b = 0, c = 0;
        while (true) {
            c = fgetc(f);
            if (c == EOF)
                break;
            if (c == '\n') {
                whitelist.push_back(strdup(buf));
                
                memset(buf, 0, b);
                b = 0;
                continue;
            }
            buf[b++] = c;
        }
        fclose(f);
    }
    
    memset(buf, 0, 1024*4);
    sprintf(buf, "%s/uzbl/cookies.txt", getenv("XDG_DATA_HOME"));
    f = fopen(buf, "w");
    
    // as a note, that link isn`t actually valid anymore.
    const char* HEAD = "    # Netscape HTTP Cookie File\n" \
              "    # http://www.netscape.com/newsref/std/cookie_spec.html\n" \
              "    # This is a generated file!  Do not edit.\n\n";
    fwrite(HEAD, 1, strlen(HEAD), f);
    
    int t = time(NULL);
    
    std::list<Cookie*> todelete;
    
    std::list<Cookie*>::iterator iter;
    for(iter = cookies.begin(); iter != cookies.end(); iter++) {
        Cookie* c = *iter;
        if (whitelist.size() != 0) {
            bool ok = false;
            for(witer = whitelist.begin(); witer != whitelist.end(); witer++) {
                if (domain_match(c->domain, *witer)) {
                    ok = true;
                    break;
                }
            }
            if (!ok)
                continue;
        }
        
        if ((c->expires < t && c->expires != 0)) {
            cookies.remove(c);
            todelete.push_back(c);
            continue;
        }
        
        memset(buf, 0, 1024*4);
        strcat(buf, c->domain);
        strcat(buf, "\t");
        if (c->domain[0] == '.')
            strcat(buf, "TRUE\t");
        else
            strcat(buf, "FALSE\t");
        strcat(buf, c->path);
        strcat(buf, "\t");
        if (c->secure == true)
            strcat(buf, "TRUE\t");
        else 
            strcat(buf, "FALSE\t");
        if (c->expires != 0) {
            char tbuf[32];
            sprintf(tbuf, "%u\t", c->expires);
            strcat(buf, tbuf);
        }
        else 
            strcat(buf, "\t");
        fwrite(buf, 1, strlen(buf), f);
        fwrite(c->key, 1, strlen(c->key), f);
        fwrite("\t", 1, 1, f);
        fwrite(c->value, 1, strlen(c->value), f);
        fwrite("\n", 1, 1, f);
    }
    
    fclose(f);
    
    for(iter = todelete.begin(); iter != todelete.end(); iter++) {
        delete *iter;
    }
    for(witer = whitelist.begin(); witer != whitelist.end(); witer++) {
        delete[] *witer;
    }
}

void CookieJar::HandleCookie(CookieRequest* req)
{
    if (!strcmp(req->Cmd(), "GET")) {
        char cookie[1024*4];
        memset(cookie, 0, 1024*4);
        printf("GET %s%s\n", req->Host(), req->Path());
        char domain[1024];
        sprintf(domain, ".%s", req->Host());
        
        int t = time(NULL);
        
        std::list<Cookie*>::iterator iter;
        for(iter = cookies.begin(); iter != cookies.end(); iter++) {
            Cookie* c = *iter;
            
            if(!endswith(domain, c->domain))
                continue;
            if(!startswith(req->Path(), c->path))
                continue;
            if (c->expires < t && c->expires != 0)
                continue;

            char tmp[1024*4];
            
            sprintf(tmp, "%s=%s; ", c->key, c->value);
            strcat(cookie, tmp);
        }
        
        cookie[strlen(cookie)-2] = '\0';
        send(req->Fd(), cookie, strlen(cookie), 0);
        if (cookie[0])
            printf("[%s]\n\n", cookie);
    }
    if (!strcmp(req->Cmd(), "PUT")) {
        printf("PUT %s%s\n[%s]\n\n", req->Host(), req->Path(), req->Data());
        
        Cookie* c = new Cookie(req->Host(), req->Data());
        if (c->path == NULL)
            c->path = strdup(req->Path());
        
        std::list<Cookie*>::iterator iter;
        int t = time(NULL);
        if (c->expires != 0) {
            if (c->expires < t) {
                for(iter = cookies.begin(); iter != cookies.end(); iter++) {
                    Cookie* ci = *iter;
                    if (!strcmp(ci->key, c->key)) {
                        delete ci;
                        cookies.remove(ci);
                        break;
                    }
                }
                return;
            }
        }
        
        bool found = false;
        
        for(iter = cookies.begin(); iter != cookies.end(); iter++) {
            if (!strcmp((*iter)->domain, c->domain)) {
                if (!strcmp((*iter)->path, c->path)) {
                    if (!strcmp((*iter)->key, c->key)) {
                        Cookie* c2 = *iter;
                        delete *iter;
                        cookies.remove(c2);
                        break;
                    }
                }
            }
        }
        
        cookies.push_back(c);
        cookies.sort(cookiesort);
    }
}


void CookieJar::AddToQueue(int cfd, char** cookie)
{
    if (strlenv(cookie) < 4)
        return;
    
    CookieRequest* c = new CookieRequest(cfd, cookie);
    
    HandleCookie(c);
    
    delete c;
}

void CookieJar::Run()
{
    timeval timeout;
    
    int writetimer = 0;
    bool needwrite = false;
    
    while (true) {
        FD_ZERO(&readfd);
        FD_SET(cookiefd, &readfd);
        timeout.tv_sec = 0;
        timeout.tv_usec = 800; // arbitrary
        select(cookiefd+1, &readfd, NULL, NULL, &timeout);
                
        if (!FD_ISSET(cookiefd, &readfd) || writetimer > 100) { // arbitrary
            if (needwrite) {
                needwrite = false;
                writetimer = 0;
                WriteFile();
            }
            continue;
        }
        
        sockaddr_un addr;
        memset(&addr, 0, sizeof(addr));
        socklen_t addrlen = sizeof(addr);
        int cfd = accept(cookiefd, (sockaddr*)&addr, &addrlen);
        
        char buf[1024*8];
        memset(buf, 0, 1024*8);
        recv(cfd, buf, 1024*8, 0);
        
        char** spl = nullsplit(buf);
        
        /*printf("buf: (%d) ", sizeof(spl));
        for(int i = 0; spl[i]; i++)
            printf("[%s] ", spl[i]);
        printf("\n");*/
        
        AddToQueue(cfd, spl);
        
        writetimer++;
        needwrite = true;
        strdelv(spl);
    }
}












