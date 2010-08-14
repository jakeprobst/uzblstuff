#include "mode.h"
#include "uzblem.h"

using namespace std;


Mode::Mode(UzblEM* e)
{
    em = e;
    
    /*command.push_back(strdup("forward_keys = 0"));
    command.push_back(strdup("keycmd_events = 1"));
    command.push_back(strdup("modcmd_updates = 1"));
    
    insert.push_back(strdup("forward_keys = 1"));
    insert.push_back(strdup("keycmd_events = 0"));
    insert.push_back(strdup("modcmd_updates = 0"));*/
}


Mode::~Mode()
{
    vector<char*>::iterator iter;
    for(iter = command.begin(); iter != command.end(); iter++)
        delete[] *iter;
    for(iter = insert.begin(); iter != insert.end(); iter++)
        delete[] *iter;
}


void Mode::ConfigChange(char** cmd)
{
    if (!strcmp(cmd[1], "default_mode")) {
        if (!strcmp(cmd[3], "command")) {
            mdefault = MODE_COMMAND;
            mode = MODE_COMMAND;
            SetMode(command);
        }
        if (!strcmp(cmd[3], "insert")) {
            mdefault = MODE_INSERT;
            mode = MODE_INSERT;
            SetMode(insert);
        }
    }
    if (!strcmp(cmd[1], "mode")) {
        if (!strcmp(cmd[3], "command")) {
            if (mode != MODE_COMMAND) {
                mode = MODE_COMMAND;
                SetMode(command);
            }
        }
        if (!strcmp(cmd[3], "insert")) {
            if (mode != MODE_INSERT) {
                mode = MODE_INSERT;
                SetMode(insert);
            }
        }
        
    }
    
    /*char a[1024];
    if (mode == MODE_COMMAND)
        strcpy(a, "EVENT [1234] MODE_CHANGE command");
    if (mode == MODE_INSERT)
        strcpy(a, "EVENT [1234] MODE_CHANGE insert");
    
    em->Command(a);*/
    
}


void Mode::SetConfig(char** cmd)
{
    char* m = strjoin(" ", cmd+2);
    if (!strcmp(cmd[1], "command"))
        command.push_back(m);
    else if (!strcmp(cmd[1], "insert"))
        insert.push_back(m);
    else
        delete[] m;
}


void Mode::KeyPress(char** cmd)
{
    if (!strcmp(cmd[1], "Escape")) {
        if (mdefault == MODE_COMMAND) {
            mode = MODE_COMMAND;
            SetMode(command);
        }
        if (mdefault == MODE_INSERT) {
            mode = MODE_INSERT;
            SetMode(insert);
        }
    }
    
    //if (mode == MODE_INSERT)
    //em->SendCommand("set keycmd = ");
}


void Mode::SetMode(std::vector<char*> v)
{
    vector<char*>::iterator iter;
    for(iter = v.begin(); iter != v.end(); iter++) {
        char c[1024];
        //sprintf(c, "%s", *iter);
        em->SendCommand(*iter);
    }
}



void Mode::Event(char** cmd)
{
    if (!strcmp(cmd[0], "MODE_CONFIG"))
        SetConfig(cmd);
    if (!strcmp(cmd[0], "VARIABLE_SET"))
        ConfigChange(cmd);
    if (!strcmp(cmd[0], "KEY_PRESS"))
        KeyPress(cmd);
}


