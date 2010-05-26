#ifndef _MODE_H_
#define _MODE_H_

#include "plugin.h"
#include <vector>


class UzblEM;

/*
class ModeConfig {
    private:
        char* name;
        char* value;
        
    public:
        ModeConfig(char* n, char* v) {name = n; value = v;};
        ~ModeConfig() {delete[] name; delete[] value;};
        
        char* GetName() {return name;};
        char* GetValue() {return value;};
};
*/

enum {
    MODE_COMMAND,
    MODE_INSERT
};


class Mode: public Plugin {
    private:
        UzblEM* em;
        
        int mdefault;
        int mode;
        std::vector<char*> command;
        std::vector<char*> insert;
    
        void ConfigChange(char**);
        void SetConfig(char**);
    
        void KeyPress(char**);
        void SetMode(std::vector<char*>);
    
    public:
        Mode(UzblEM*);
        ~Mode();
               
        void Event(char** );
};















#endif /* _MODE_H_ */
