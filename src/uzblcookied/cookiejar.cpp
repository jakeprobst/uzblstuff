#include "cookiejar.h"
#include "util.h"

#include <fstream>

//extern Context *ctx;
const size_t BUFSIZE = 1024*8 + 2;

const char* SOCKETFILE = "uzbl/cookie_daemon_socket";

bool domain_match(const char* a, const char* b)
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

    cookiefd = socket(AF_UNIX, SOCK_STREAM, 0);

    sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    sprintf(addr.sun_path, "%s/%s", xdgCacheHome(&xdg), SOCKETFILE);
    
    unlink(addr.sun_path);
    
    bind(cookiefd, (sockaddr*)&addr, sizeof(addr));
    listen(cookiefd, 10);
    
    maxfd = cookiefd;
    FD_ZERO(&masterfd);
    FD_SET(cookiefd, &masterfd);
    
    LoadFile();
}

CookieJar::~CookieJar()
{
    xdgWipeHandle(&xdg);
    close(cookiefd);
}

void CookieJar::LoadFile()
{
    std::string path = xdgDataHome(&xdg) + std::string("/uzbl/cookies.txt");
    std::ifstream file(path.c_str());
    if (!file.good())
        return;

    std::string line;
    while (file.good()) {
        line.clear();
        getline(file, line);

        // Ignore this line if it's a comment or empty
        if (line.empty())
            continue;

        size_t pos = line.find_first_not_of(' ');
        if (pos != std::string::npos && line[pos] == '#')
            continue;

        cookies.insert(Cookie(line.c_str()));
    }
}

/* Reads whitelist file.
 * returns:
 *   -1     if whitelist file does not exist
 *   number of entries otherwise */
int CookieJar::ReadWhitelist(std::string &path, std::set<std::string> &whitelist) {
    std::ifstream wl(path.c_str());

    if (!wl.good()) return -1;

    while (wl.good()) {
        std::string line;
        getline(wl, line);
        whitelist.insert(line);
    }
    wl.close();

    return whitelist.size();
}


bool CookieJar::IsInWhitelist(const Cookie& c)
{
    std::set<std::string> whitelist;

    std::string path = xdgConfigHome(&xdg) + std::string("/uzbl/cookie_whitelist");

    int whitelisttype = ReadWhitelist(path, whitelist);

    // 0 = empty, -1 = nonexistant
    if (whitelisttype == -1) {
        ctx->log(1, "whitelist doesnt exist, accepting all cookies.");
        return true;
    }
    if (whitelisttype == 0) {
        ctx->log(1, "Whitelist is empty, not accepting cookies.");
        return false;
    }
    
    std::set<std::string>::iterator it;
    for(it = whitelist.begin(); it != whitelist.end(); it++) {
        if (domain_match(c.domain, it->c_str())) {
            return true;
        }
    }
    return false;
}


void CookieJar::WriteFile()
{
    std::set<std::string> storelist;

    std::string path = xdgConfigHome(&xdg) + std::string("/uzbl/cookie_storelist");

    int storelisttype = ReadWhitelist(path, storelist);

    /* return value 0 means that whitelist file exists, but is empty. It means
     * we shoud not write any cookie */
    if (storelisttype == 0) {
        ctx->log(1, "Storelist is empty, not writing cookies file.");
        return;
    }

    path = xdgDataHome(&xdg) + std::string("/uzbl/cookies.txt");
    std::ofstream file(path.c_str());
    if (!file.good()) {
        ctx->log(1, "Cannot write cookies file.");
        return;
    }

    // as a note, that link isn`t actually valid anymore.
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
        /* if whitelisttype == -1, whitelistfile does not exist, so we should
         * accept all cookies */
        if (storelisttype > 0) {
            bool ok = false;
            std::set<std::string>::iterator it;
            for(it = storelist.begin(); it != storelist.end(); it++) {
                if (domain_match(c.domain, it->c_str())) {
                    ok = true;
                    break;
                }
            }
            if (!ok)
                continue;
        }

        file << c.data << "\n";
    }
    ctx->log(1, "Cookies file wrote.");
}

void CookieJar::HandleCookie(Cookie req)
{
    for(int i = 0; i < maxfd; i++) {
        if (FD_ISSET(i, &masterfd)) {
            if (!IsInWhitelist(req)) {
                return;
            }
            char b[BUFSIZE];
            sprintf(b, "add_cookie %s\n", req.data);
            send(i, b, strlen(b), 0);
        }
    }

    std::pair<cookieSet::iterator, bool> res = cookies.insert(req);
    if (!res.second) {
        // There was already another cookie with is equal to this one,
        // remove the other one and then insert again.
        // If this new cookie is already expired, the normal expiry machine
        // will kick in and remove it later on.
        cookies.erase(res.first);
        cookies.insert(req);
    }
}


void CookieJar::AddToQueue(int cfd, char* cookie)
{
    int l = 0, i = 0;;
    for(i = 0; l < 2; i++) {
        if (cookie[i] == ' ')
            l++;
    }
    if (strncmp(cookie+i, "ADD_COOKIE", 10) == 0) {
        if (cookie[strlen(cookie)-1] == '\n')
            cookie[strlen(cookie)-1] = '\0';
        Cookie c(cookie+i+strlen("ADD_COOKIE "));
        HandleCookie(c);
        needwrite = true;
    }
}

void CookieJar::Run()
{
    timeval timeout;
    
    while (ctx->running) {
        if (ctx->writerequest) {
            ctx->writerequest = false;
            ctx->log(1, "Write request received. Trying to write cookies file.");
            WriteFile();
        }

        FD_ZERO(&readfd);
        readfd = masterfd;
        timeout.tv_sec = 3; // arbitrary
        timeout.tv_usec = 0; 
        int i = select(maxfd+1, &readfd, NULL, NULL, &timeout);

        if (i < 0) {
            ctx->perror("select");
            continue;
        }

        if (FD_ISSET(cookiefd, &readfd)) {
            sockaddr_un addr;
            memset(&addr, 0, sizeof(addr));
            socklen_t addrlen = sizeof(addr);
            int cfd = accept(cookiefd, (sockaddr*)&addr, &addrlen);
            if (cfd < 0) {
                ctx->perror("accept");
                continue;
            }

            FD_SET(cfd, &masterfd);
            if (cfd > maxfd)
                maxfd = cfd;
            FD_CLR(cookiefd, &readfd);
        }
        
        for(int sfd = 0; sfd <= maxfd; sfd++) {
            if (FD_ISSET(sfd, &readfd)) {
                char buf[BUFSIZE];
                memset(buf, 0, BUFSIZE);
                int ret = recv(sfd, buf, BUFSIZE, 0);
                if (ret <= 0) {
                    if (ret == 0)
                        ctx->log(2, std::string("Client hung up"));
                    else
                        ctx->perror("recv");
                    close(sfd);
                    FD_CLR(sfd, &masterfd);
                    continue;
                }
                
                char** spl = strsplit(buf, '\n');
                
                for(int i = 0; spl[i]; i++)
                    AddToQueue(sfd, spl[i]);
                
                strdelv(spl);
            }
        }
        
        if (!ctx->memory_mode) {
            if (needwrite) {
                needwrite = false;
                ctx->log(1, std::string("Write request received. Trying to write"));
                WriteFile();
            }
            continue;
        }
    }
}












