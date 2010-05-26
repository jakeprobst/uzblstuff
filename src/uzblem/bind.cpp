#include "bind.h"
#include "str.h"
#include "uzblem.h"

#include <vector>

using namespace std;

Bind::Bind(UzblEM* e)
{
    em = e;
    kindex = 0;
    kselect = 0;
    input = true;
    memset(modcmd, 0, 256);
    memset(keycmd, 0, 1024);
}

Bind::~Bind()
{
    vector<Binding*>::iterator iter;
    for(iter = bindings.begin(); iter != bindings.end(); iter++) {
        delete *iter;
    }
    vector<char*>::iterator iter2;
    for(iter2 = ignore.begin(); iter2 != ignore.end(); iter2++) {
        delete[] *iter;
    }
}

char* Bind::FixModkey(char* key)
{
    char* mk = new char[256];
    memset(mk, 0, 256);
    
    if (strcontains(key, "_L") || strcontains(key, "_R"))
        strncpy(mk, key, strlen(key)-2);
    else
        strncpy(mk, key, strlen(key));
        
    if (!strcmp(mk, "Control")) {
        memset(mk, 0, 256);
        strncpy(mk, "Ctrl", 4);
    }
    
    return mk;
}

void Bind::ModeChange(char** cmd)
{
    if (!strcmp(cmd[1], "command"))
        input = true;
    if (!strcmp(cmd[1], "insert"))
        input = false;
}

void Bind::SetKeyCmd()
{
    char c[1024];
    memset(c, 0, 1024);
    
    if (strlen(modcmd) == 0) {
        if (keycmd[kselect]) {
            char pre[strlen(keycmd)];
            strncpy(pre, keycmd, strlen(keycmd));
            pre[kselect] = '\0';
            
            sprintf(c, "set keycmd = @[%s]@<span @cursor_style>@[%c]@</span>@[%s]@\n", pre, keycmd[kselect], keycmd+kselect+1);
        }
        else {
            sprintf(c, "set keycmd = @[%s]@\n", keycmd);
        }
    }
    else {
        sprintf(c, "set keycmd = @[%s%s]@\n", modcmd, keycmd);
    }
    
    char* c2  = strreplace(c, "\\", "\\\\", -1);

    em->SendCommand(c2);
    delete[] c2;
}


void Bind::TryAndExec()
{
    vector<Binding*>::iterator iter;
    for(iter = bindings.begin(); iter != bindings.end(); iter++) {
        if ((*iter)->IsArg()) {
            char* key = (*iter)->GetKey();
            bool exec = false;
            
            int len = charat(keycmd, ' ');
            if (!strncmp(key, keycmd, len) && strlen(key) == len)
                exec = true;
            else if (!strncmp(key, keycmd, strlen(key)) && (*iter)->IsSpace() == 0)
                exec = true;

            if (exec) {
                char* cmd = strreplace((*iter)->GetCmd(), "%s", 
                                       keycmd+strlen(key)+(*iter)->IsSpace(), 1);
                //printf("cmd: %s\n", cmd);
                em->SendCommand(cmd);
                
                delete[] cmd; 
                memset(keycmd, 0, 1024);
                kindex = 0;
                kselect = 0;
            }
        }
    }
}


void Bind::EventBind(char** cmd, bool mkb)
{
    char* key;
    char* value;
    int klen = 1;
    int vlen = 1;
    bool arg = false;
    bool space = 1;
    int i;
    
    for(i = 1; cmd[i] != NULL; ++i) {
        if (cmd[i][0] == '=')
            break;
        if (cmd[i][strlen(cmd[i])-1] == '_') {
            if (strlen(cmd[i]) != 1)
                space = 0;
            arg = true;
            klen += strlen(cmd[i])-2;
            continue;
        }
        klen += strlen(cmd[i]) +1;
    }
    i++;
    for(; cmd[i] != NULL; ++i) {
        vlen += strlen(cmd[i]) +1;
    }
    
    key = new char[klen];
    memset(key, 0, klen);
    value = new char[vlen];
    memset(value, 0, vlen);
    
    for(i = 1; cmd[i] != NULL; ++i) {
        if (cmd[i][0] == '=')
            break;
        if (cmd[i][0] == '_')
            continue;
        strncat(key, cmd[i], klen);
        strncat(key, " ", klen);
    }
    key[strlen(key)-1] = '\0';
    
    i++;
    for(; cmd[i] != NULL; ++i) {
        strncat(value, cmd[i], vlen);
        strncat(value, " ", vlen);
    }
    value[strlen(value)-1] = '\0';
    
    Binding* b;
    b = new Binding(key, value, arg, space, mkb);
    
    bindings.push_back(b);
}

void Bind::EventKeyPress(char** cmd)
{
    if (input == false)
        return;
    for(int i = 0; i < exec.size(); i++) {
        if (!strcmp(cmd[1], exec[i])) {
            TryAndExec();
            return;
        }
    }
    if (!strcmp(cmd[1], "Escape")) {
        memset(keycmd, 0, 1024);
        memset(modcmd, 0, 256);
        kindex = 0;
        kselect = 0;
    }
    else if (!strcmp(cmd[1], "Shift_R") || !strcmp(cmd[1], "Shift_L")) {
        // nothing!
    }
    else if (!strcmp(cmd[1], "Left")) {
        kselect--;
        if (kselect < 0)
            kselect = 0;
    }
    else if (!strcmp(cmd[1], "Right")) {
        kselect++;
        if (kselect > kindex)
            kselect = kindex;
    }
    else if (!strcmp(cmd[1], "BackSpace")) {
        if (kselect > 0) {
            for(int i = kselect; i < kindex; i++) {
                keycmd[i-1] = keycmd[i];
            }
            keycmd[kindex-1] = '\0';
            kindex--;
            kselect--;
        }
    }
    else if (!strcmp(cmd[1], "Delete")) {
        if (keycmd[kselect] == '\0')
            return;
        for(int i = kselect; i < kindex; i++) {
            keycmd[i] = keycmd[i+1];
        }
        keycmd[kindex] = '\0';
        kindex--;
    }
    else if (!strcmp(cmd[1], "space")) { // why is this lowercase?
        if (kindex == 0)
            return;
        for(int i = kindex; i != kselect-1; i--) {
            keycmd[i+1] = keycmd[i];
        }
        keycmd[kselect] = ' ';
        kindex++;
        kselect++;
    }
    else if (strlen(cmd[1]) != 1) {
        char* mk = FixModkey(cmd[1]);
        int len = strlen(mk);
        
        if (strcontains(modcmd, mk)) {
            delete[] mk;
            return;
        }
        
        vector<char*>::iterator iter;
        for(iter = ignore.begin(); iter != ignore.end(); iter++) {
            if (!strncmp((*iter)+1, mk, strlen(mk))) { // +1 = < // +4 = &lt;
                delete[] mk;
                return;
            }
        }
        
        strncat(modcmd, "<", 1);
        strncat(modcmd, mk, len);
        strncat(modcmd, ">", 1);
        
        delete[] mk;
    }
    else {
        vector<char*>::iterator iter;
        for(iter = ignore.begin(); iter != ignore.end(); iter++) {
            if (strcmp((*iter), cmd[1]) == 0) {
                return;
            }
        }
                
        for(int i = kindex; i != kselect-1; i--) {
            keycmd[i+1] = keycmd[i];
        }
        keycmd[kselect] = cmd[1][0];
        kindex++;
        kselect++;
    }
        
    char kc[1024];
    sprintf(kc, "%s%s", modcmd, keycmd);
    
    vector<Binding*>::iterator iter;
    for(iter = bindings.begin(); iter != bindings.end(); iter++) {
        if ((*iter)->Match(kc)) {
            if (!(*iter)->IsArg()) {
                em->SendCommand((*iter)->GetCmd());
                
                memset(modcmd, 0, 256);
                memset(keycmd, 0, 1024);
                kindex = 0;
                kselect = 0;
            }
        }
        else if ((*iter)->IsModkeyBind()) {
            if (!strcmp((*iter)->GetKey(), modcmd)) {
                em->SendCommand((*iter)->GetCmd());
                memset(modcmd, 0, 256);
            }
        }
    }
    
    SetKeyCmd();
}


void Bind::EventKeyRelease(char** cmd)
{   
    if (strlen(cmd[1]) <= 1)
        return;
    
    char* mk = FixModkey(cmd[1]);
    char mk2[256];
        
    sprintf(mk2, "<%s>", mk);
    char* s = strreplace(modcmd, mk2, "", -1);
    
    memset(modcmd, 0, 256);
    strcpy(modcmd, s);
    
    SetKeyCmd();
    
    delete[] mk;
    delete[] s;
}

void Bind::EventIgnoreKey(char** cmd)
{
    ignore.push_back(strdup(cmd[1]));
}

void Bind::EventExecKey(char** cmd)
{
    //printf("exec added: %s\n", cmd[1]);
    exec.push_back(strdup(cmd[1]));
}

void Bind::EventKeycmdInsert(char** cmd)
{
    char* str = strjoin(" ", cmd+1);
    int len = strlen(str);
    
    char kcmd[1024];
    memcpy(kcmd, keycmd, 1024);
    
    char pre[strlen(keycmd)];
    strcpy(pre, keycmd);
    pre[kselect] = '\0';
    sprintf(keycmd, "%s%s%s", pre, str, kcmd+kselect);
    
    kindex += len;
    kselect += len;
    delete[] str;
    
}


void Bind::Event(char** cmd)
{
    if (!strcmp(cmd[0], "BIND"))
        EventBind(cmd);
    if (!strcmp(cmd[0], "MODKEY_BIND"))
        EventBind(cmd, true);
    if (!strcmp(cmd[0], "KEY_PRESS"))
        EventKeyPress(cmd);
    if (!strcmp(cmd[0], "KEY_RELEASE"))
        EventKeyRelease(cmd);
    if (!strcmp(cmd[0], "MODE_CHANGE"))
        ModeChange(cmd);
    if (!strcmp(cmd[0], "IGNORE_KEY"))
        EventIgnoreKey(cmd);
    if (!strcmp(cmd[0], "EXEC_KEY"))
        EventExecKey(cmd);
    if (!strcmp(cmd[0], "KEYCMD_INSERT"))
        EventKeycmdInsert(cmd);
}










