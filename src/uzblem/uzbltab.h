#ifndef _UZBLTAB_H_
#define _UZBLTAB_H_

#include "plugin.h"

class UzblEM;

class UzblTab : public Plugin {
    private:
        UzblEM* em;
        char fifotab[256];
    
        void TitleChanged(char**);
        void LoadStart(char**);
        void InstanceExit(char**);
        void SetFifoTab(char**);
        
    
    public:
        UzblTab(UzblEM*);
        ~UzblTab();
        
        
        void Event(char**);
    
};











#endif /* _UZBLTAB_H_ */
