#ifndef _BIND_H_
#define _BIND_H_

#include <vector>
#include <string.h>

#include "plugin.h"

class UzblEM;

class Binding {
    private:
        char* key;
        char* cmd;
        bool arg;
        bool mkb;
        int space;
        
    public:
        Binding(char* k, char* c, bool a, int s, bool m = false)
                {key = k; cmd = c; arg = a; space = s; mkb = m;};
        ~Binding() {delete[] key; delete[] cmd;};
        
        bool Match(char* k) {return !strcmp(key, k);};
        char* GetKey() {return key;};
        char* GetCmd() {return cmd;};
        bool IsArg() {return arg;};
        int IsSpace() {return space;};
        bool IsModkeyBind() {return mkb;};
};


class Bind : public Plugin {
    private:
        UzblEM* em;
        
        int kselect;
        int kindex;
        bool input;
        //char modcmd[256];
        char keycmd[1024];
        
        std::vector<Binding*> bindings;
        std::vector<char*> ignore;
        std::vector<char*> exec;
        std::vector<char*> modkeys;
        
        char* FixModkey(char*);
        
        void ModeChange(char**);
        void SetKeyCmd();
        void TryAndExec();
        
        void EventBind(char**, bool mkb = false);
        void EventKeyPress(char**);
        void EventKeyRelease(char**);
        void EventIgnoreKey(char**);
        void EventExecKey(char**);
        void EventKeycmdInsert(char**);
    
    public:
        Bind(UzblEM*);
        ~Bind();
        
        void Event(char**);
};




#endif /* _BIND_H_ */
