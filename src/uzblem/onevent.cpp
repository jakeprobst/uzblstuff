#include "uzblem.h"
#include "onevent.h"
#include "str.h"

using namespace std;


OnEvent::OnEvent(UzblEM* e)
{
    em = e;
}

OnEvent::~OnEvent()
{
    vector<EventCallback*>::iterator iter;
    for(iter = events.begin(); iter != events.end(); iter++)
        delete *iter;
}


void OnEvent::RegisterEvent(char** cmd)
{
    char* c = strjoin(" ", cmd+2);
    
    char* ev = new char[strlen(cmd[1])+1];
    strncpy(ev, cmd[1], strlen(cmd[1])+1);
    
    EventCallback* e = new EventCallback(ev, c);

    events.push_back(e);
}


void OnEvent::CheckEvents(char** cmd)
{
    
    vector<EventCallback*>::iterator iter;
    for(iter = events.begin(); iter != events.end(); iter++) {
        if ((*iter)->Match(cmd[0])) {
            if (cmd[1] && strcontains((*iter)->GetCmd(), "%s")) {
                char* arg = strjoin(" ", cmd+1);
                char* c = strreplace((*iter)->GetCmd(), "%s", arg, -1);
                em->SendCommand(c);
                delete[] arg;
                delete[] c;
            }
            else {
                em->SendCommand((*iter)->GetCmd());
            }
        }
    }
}


void OnEvent::Event(char** cmd)
{
    if (!strcmp(cmd[0], "ON_EVENT"))
        RegisterEvent(cmd); 
    else
        CheckEvents(cmd);    
}



























