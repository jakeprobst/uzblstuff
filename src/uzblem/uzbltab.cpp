#include "uzbltab.h"
#include "uzblem.h"



UzblTab::UzblTab(UzblEM* e)
{
    em = e;
}


UzblTab::~UzblTab()
{
}

void UzblTab::TitleChanged(char** cmd)
{
    char s[1024];
    //char* t = strjoin(" ", cmd+1);
    sprintf(s, "tabtitle %s %s\n", em->GetName(), cmd[1]);
    //printf(s);
    
    int fd;
    if (!access(fifotab, F_OK)) {
        fd = open(fifotab, O_WRONLY|O_NONBLOCK);
        write(fd, s, strlen(s));
        close(fd);
    }
    
    //delete[] t;
}

void UzblTab::LoadStart(char** cmd)
{
    char s[1024];
    //char* t = strjoin(" ", cmd+1);
    sprintf(s, "taburi %s %s\n", em->GetName(), cmd[1]);
    
    int fd;
    if (!access(fifotab, F_OK)) {
        fd = open(fifotab, O_WRONLY|O_NONBLOCK);
        write(fd, s, strlen(s));
        close(fd);
    }
    
}

void UzblTab::InstanceExit(char** cmd)
{
    char s[1024];
    sprintf(s, "close %s\n", em->GetName());
    
    int fd;
    if (!access(fifotab, F_OK)) {
        fd = open(fifotab, O_WRONLY|O_NONBLOCK);
        write(fd, s, strlen(s));
        close(fd);
    }
}

void UzblTab::SetFifoTab(char** cmd)
{
    printf("fifotab set: %s!\n", cmd[1]);
    strncpy(fifotab, cmd[1], 256);
}


void UzblTab::Event(char** cmd)
{
    if (!strcmp(cmd[0], "TITLE_CHANGED")) {
        TitleChanged(cmd);
    }
    if (!strcmp(cmd[0], "LOAD_COMMIT")) {
        LoadStart(cmd);
    }
    if (!strcmp(cmd[0], "FIFOTAB")) {
        SetFifoTab(cmd);
    }
    if (!strcmp(cmd[0], "INSTANCE_EXIT")) {
        InstanceExit(cmd);
    }
}





