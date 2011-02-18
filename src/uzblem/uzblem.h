#ifndef _UZBLEM_H_
#define _UZBLEM_H_

#include <vector>

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>

#include "str.h"
#include "plugin.h"

const int BUFSIZE = 1024*8;

class UzblEM {
    private:
        int sockfd;
        
        char* sockpath;
        char* name;
        
        std::vector<Plugin*> plugins;
        
        void InitPlugins();
    
        void EventHandler(char**);
        
    public:
        UzblEM(char*);
        ~UzblEM();
        
        char* GetSocket() {return sockpath;};
        char* GetName() {return name;};
        
        void Command(char*);
        void AttachPlugin(Plugin*);
        void SendCommand(char*);
        
        void Run();
};







#endif /* _UZBLEM_H_ */
