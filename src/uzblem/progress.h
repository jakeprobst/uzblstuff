#ifndef _PROGRESS_H_
#define _PROGRESS_H_

#include "plugin.h"

class UzblEM;

class Progress: public Plugin {
    private:
        UzblEM* em;
        
        int width;
        char* done;
        char* pending;
        char* format;
        char* spinner;
        char* sprites;
        int updates;
        int progress;
        
        void ProgressConfig(char**);
        void ResetProgress();
        void UpdateProgress(char**);
    
    public:
        Progress(UzblEM*);
        ~Progress();
        
        
        void Event(char**);
};













#endif /* _PROGRESS_H_ */
