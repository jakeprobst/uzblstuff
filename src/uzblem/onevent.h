#ifndef _ONEVENT_H_
#define _ONEVENT_H_

#include "plugin.h"
#include <vector>

class UzblEM;

class EventCallback {
    private:
        char* event;
        char* cmd;
        
    public:
        EventCallback(char* e, char* c) {event = e; cmd = c;};
        ~EventCallback() {delete[] event; delete[] cmd;};
        
        bool Match(char* e) {return !strcmp(event, e);};
        char* GetCmd() {return cmd;};
};


class OnEvent : public Plugin {
    private:
        UzblEM* em;
        
        std::vector<EventCallback*> events;
    
        void RegisterEvent(char**);
        void CheckEvents(char**);
    
    
    public:
        OnEvent(UzblEM*);
        ~OnEvent();
        
        
         void Event(char**);
};










#endif /* _ONEVENT_H_ */
