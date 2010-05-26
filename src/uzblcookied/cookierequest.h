#ifndef _COOKIEREQUEST_H_
#define _COOKIEREQUEST_H_




class CookieRequest {
    private:
        int fd;
        char* cmd;
        char* protocol;
        char* host;
        char* path;
        char* data;
    
    public:
        CookieRequest(int,char**);
        ~CookieRequest();
        
        char* Cmd() {return cmd;};
        char* Protocol() {return protocol;};
        char* Host() {return host;};
        char* Path() {return path;};
        char* Data() {return data;};
        int Fd() {return fd;};
};















#endif /* _COOKIEREQUEST_H_ */
