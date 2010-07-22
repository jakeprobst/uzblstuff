#include "cookiejar.h"
#include "util.h"

//extern Context *ctx;

const char* SOCKETFILE = "uzbl/cookie_daemon_socket";

bool domain_match(char* a, char* b)
{
    if (startswith(b, ".") && endswith(a, b))
        return true;
    if (!startswith(b, ".") && !strcmp(a, b))
        return true;
    return false;
}


CookieJar::CookieJar(Context* c)
        : cookies(cookiesort)
{
    if (!xdgInitHandle(&xdg)) throw "Unable to initialize XDG handle.";

    ctx = c;

    cookiefd = socket(AF_UNIX, SOCK_SEQPACKET, 0);
    //printf("socket: %s\n", strerror(errno));

    sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    sprintf(addr.sun_path, "%s/%s", xdgCacheHome(&xdg), SOCKETFILE);
    
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
    xdgWipeHandle(&xdg);
    close(cookiefd);
}

void CookieJar::LoadFile()
{
    char buf[1024*4];
    memset(buf, 0, 1024*4);
    sprintf(buf, "%s/uzbl/cookies.txt", xdgDataHome(&xdg));
    
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
            cookies.insert(Cookie(buf));
            
            memset(buf, 0, b);
            b = 0;
            continue;
        }
        
        buf[b++] = c;
    }
    fclose(f);
}

void CookieJar::WriteFile()
{
    char buf[1024*4];
    memset(buf, 0, 1024*4);
    
    std::list<char*>::iterator witer;
    std::list<char*> whitelist;
    sprintf(buf, "%s/uzbl/cookie_whitelist", xdgDataHome(&xdg));
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
    sprintf(buf, "%s/uzbl/cookies.txt", xdgDataHome(&xdg));
    f = fopen(buf, "w");
    
    // as a note, that link isn`t actually valid anymore.
    const char* HEAD = "    # Netscape HTTP Cookie File\n" \
              "    # http://www.netscape.com/newsref/std/cookie_spec.html\n" \
              "    # This is a generated file!  Do not edit.\n\n";
    fwrite(HEAD, 1, strlen(HEAD), f);
    
    int t = time(NULL);
    
    cookieSet::iterator iter;

    // First get rid of all expired cookies
    for(iter = cookies.begin(); iter != cookies.end(); ) {
        const Cookie& c = *iter;

        if ((c.expires < t && c.expires != 0)) {
            // We first grab the iterator to the next item in the set.
            // set gurantees that this iterator stays valid after an erase().
            cookieSet::iterator next = iter;
            ++next;
            cookies.erase(iter);
            iter = next;
        } else
            ++iter;
    }

    for(iter = cookies.begin(); iter != cookies.end(); iter++) {
        const Cookie& c = *iter;
        if (whitelist.size() != 0) {
            bool ok = false;
            for(witer = whitelist.begin(); witer != whitelist.end(); witer++) {
                if (domain_match(c.domain, *witer)) {
                    ok = true;
                    break;
                }
            }
            if (!ok)
                continue;
        }
        
        memset(buf, 0, 1024*4);
        strcat(buf, c.domain);
        strcat(buf, "\t");
        if (c.domain[0] == '.')
            strcat(buf, "TRUE\t");
        else
            strcat(buf, "FALSE\t");
        strcat(buf, c.path);
        strcat(buf, "\t");
        if (c.secure == true)
            strcat(buf, "TRUE\t");
        else 
            strcat(buf, "FALSE\t");
        if (c.expires != 0) {
            char tbuf[32];
            sprintf(tbuf, "%u\t", c.expires);
            strcat(buf, tbuf);
        }
        else 
            strcat(buf, "\t");
        fwrite(buf, 1, strlen(buf), f);
        fwrite(c.key, 1, strlen(c.key), f);
        fwrite("\t", 1, 1, f);
        fwrite(c.value, 1, strlen(c.value), f);
        fwrite("\n", 1, 1, f);
    }
    
    fclose(f);
    
    for(witer = whitelist.begin(); witer != whitelist.end(); witer++) {
        delete[] *witer;
    }
}

void CookieJar::HandleCookie(CookieRequest* req)
{
    if (!strcmp(req->Cmd(), "GET")) {
        std::string cookie;
        ctx->log(2, std::string("GET ")+req->Host()+req->Path());
        char domain[1024];
        sprintf(domain, ".%s", req->Host());
        
        int t = time(NULL);
        
        cookieSet::iterator iter;
        for(iter = cookies.begin(); iter != cookies.end(); iter++) {
            const Cookie& c = *iter;
            
            if(!endswith(domain, c.domain))
                continue;
            if(!startswith(req->Path(), c.path))
                continue;
            if (c.expires < t && c.expires != 0)
                continue;

            cookie += c.key;
            cookie += "=";
            cookie += c.value;
            cookie += "; ";
        }

        // Strip away the last "; "
        if (!cookie.empty())
            cookie = cookie.substr(0, cookie.length() - 2);

        send(req->Fd(), cookie.c_str(), cookie.length(), 0);
        if (!cookie.empty())
            ctx->log(2, std::string("[")+cookie+"]");
    }
    if (!strcmp(req->Cmd(), "PUT")) {
        ctx->log(2, std::string("PUT ")+req->Host()+req->Path()+": "+req->Data());
        
        Cookie c(req->Host(), req->Data());
        if (c.path == NULL)
            c.path = strdup(req->Path());

        std::pair<cookieSet::iterator, bool> res = cookies.insert(c);
        if (!res.second) {
            // There was already another cookie with is equal to this one,
            // remove the other one and then insert again.
            // If this new cookie is already expired, the normal expiry machine
            // will kick in and remove it later on.
            cookies.erase(res.first);
            cookies.insert(c);
        }
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
    
    while (ctx->running) {
        FD_ZERO(&readfd);
        FD_SET(cookiefd, &readfd);
        timeout.tv_sec = 3; // arbitrary
        timeout.tv_usec = 0; 
        int i = select(cookiefd+1, &readfd, NULL, NULL, &timeout);

        if (i < 0) {
            ctx->perror("select");
            continue;
        }
        
        if (!ctx->memory_mode) {
            if (!FD_ISSET(cookiefd, &readfd) || writetimer > 100) { // arbitrary
                if (needwrite) {
                    needwrite = false;
                    writetimer = 0;
                    WriteFile();
                    ctx->log(1, std::string("Cookies file wrote"));
                }
                continue;
            }
        }
        
        sockaddr_un addr;
        memset(&addr, 0, sizeof(addr));
        socklen_t addrlen = sizeof(addr);
        int cfd = accept(cookiefd, (sockaddr*)&addr, &addrlen);
        if (cfd < 0) {
            ctx->perror("accept");
            continue;
        }
        
        size_t bufSize = 1024*8 + 2;
        char buf[bufSize];
        // -2 to make sure there we can add two null bytes at the end
        int ret = recv(cfd, buf, bufSize - 2, 0);
        if (ret <= 0) {
            if (ret == 0)
                ctx->log(2, std::string("Client hung up"));
            else
                ctx->perror("recv");
            close(cfd);
            continue;
        }
        buf[ret] = buf[ret + 1] = '\0';
        
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












