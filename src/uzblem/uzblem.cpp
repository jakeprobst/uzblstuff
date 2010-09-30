#include "uzblem.h"

#include <stdlib.h>
#include <unistd.h>

// plugins
#include "bind.h"
#include "onevent.h"
#include "mode.h"
#include "uzbltab.h"
#include "progress.h"

using namespace std;

UzblEM::UzblEM(char* sock)
{
    unlink(sock);
    
    sockaddr_un local;
    int servfd = socket(AF_UNIX, SOCK_STREAM, 0);
    local.sun_family = AF_UNIX;
    strcpy(local.sun_path, sock);
    int len = strlen(local.sun_path) + sizeof(local.sun_family);
    
    bind(servfd, (sockaddr*)&local, len);
    listen(servfd, 0);
    
    switch (fork()) {
        case 0:
            
            break;
        case -1:
            perror("fork");
            exit(1);
        default:
            exit(0);
    }
    
    
    sockfd =  accept(servfd, NULL, 0);    
    sockpath = strdup(sock);
    name = NULL;
    
    InitPlugins();
}

UzblEM::~UzblEM()
{
    vector<Plugin*>::iterator piter;
    for(piter = plugins.begin(); piter != plugins.end(); piter++) {
        delete *piter;
    }
    
    unlink(sockpath);
    
    if (sockpath) delete[] sockpath;
    if (name) delete[] name;

    close(sockfd);
}

void UzblEM::AttachPlugin(Plugin* pl)
{
    //printf("attaching %s\n", evt);
    //Event* event = new Event(evt, pl);
    //events.push_back(event);
    
    plugins.push_back(pl);
}

void UzblEM::InitPlugins()
{
    AttachPlugin(new Bind(this));
    AttachPlugin(new OnEvent(this));
    AttachPlugin(new Mode(this));
    AttachPlugin(new UzblTab(this));
    AttachPlugin(new Progress(this));
}


void UzblEM::EventHandler(char** cmd)
{
    vector<Plugin*>::iterator iter;
    for(iter = plugins.begin(); iter != plugins.end(); iter++) {
        (*iter)->Event(cmd);
    }
}


void UzblEM::Command(char* c)
{
    char** cmd = strsplit(c, ' ');
    
    
    if (!name) {
        name = new char[strlen(cmd[1])];
        sscanf(cmd[1], "[%[^]]]", name);
    }
    
    if (!strcmp(cmd[0], "EVENT")) {
        EventHandler(cmd+2);
    }
    
    strdelv(cmd);
}


void UzblEM::SendCommand(char* c)
{
    int s = 0, len = strlen(c);
    while (s < len) {
        s += send(sockfd, c+s, len-s, 0);
    }
    if (c[strlen(c)] != '\n')
        send(sockfd, "\n", 1, 0);
}



void UzblEM::Run()
{
    int sb = 0;
    char sbuf[BUFSIZE];
    memset(sbuf, 0, BUFSIZE);
    while (true) {
        int r = read(sockfd, sbuf+sb, 1);
        if (r == 0)
            return;
        
        if (sbuf[sb] == '\n') {
            sbuf[sb] = '\0';
            Command(sbuf);
            memset(sbuf, 0, BUFSIZE);
            sb = -1;
        }
        
        sb++;
    }
}








