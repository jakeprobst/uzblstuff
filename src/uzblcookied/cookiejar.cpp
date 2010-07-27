#include "cookiejar.h"
#include "util.h"

#include <fstream>

//extern Context *ctx;

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

void CookieJar::WriteFile()
{
    std::set<std::string> whitelist;

    std::string path = xdgConfigHome(&xdg) + std::string("/uzbl/cookie_whitelist");

	int whitelisttype = ReadWhitelist(path, whitelist);

	/* return value 0 means that whitelist file exists, but is empty. It means
	 * we shoud not write any cookie */
	if (whitelisttype == 0) {
		ctx->log(1, "Whitelist is empty, not writing cookies file.");
		return;
	}

    path = xdgDataHome(&xdg) + std::string("/uzbl/cookies.txt");
    std::ofstream file(path.c_str());
    if (!file.good()) {
		ctx->log(1, "Cannot write cookies file.");
        return;
	}

    // as a note, that link isn`t actually valid anymore.
    file <<   "    # Netscape HTTP Cookie File\n" \
              "    # http://www.netscape.com/newsref/std/cookie_spec.html\n" \
              "    # This is a generated file!  Do not edit.\n\n";

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
        if (whitelisttype > 0) {
            bool ok = false;
    		std::set<std::string>::iterator it;
            for(it = whitelist.begin(); it != whitelist.end(); it++) {
                if (domain_match(c.domain, it->c_str())) {
                    ok = true;
                    break;
                }
            }
            if (!ok)
                continue;
        }
        
        file << c.domain << "\t";
        if (c.domain[0] == '.')
            file << "TRUE\t";
        else
            file << "FALSE\t";
        file << c.path << "\t";
        if (c.secure == true)
            file << "TRUE\t";
        else
            file << "FALSE\t";
        if (c.expires != 0)
            file << c.expires;
        file << "\t";
        file << c.key << "\t" << c.value << "\n";
    }
	ctx->log(1, "Cookies file wrote.");
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












