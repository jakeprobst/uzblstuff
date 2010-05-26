#ifndef _UZBLTABEM_H_
#define _UZBLTABEM_H_

#include <gtk/gtk.h>

#include "uzblinstance.h"

class UzblTab {
    private:
        bool running;
        int currenttab;
        int totaltabs;
        int fifocount;
        
        GtkWindow* window;
        GtkLabel* tablist;
        GtkNotebook* notebook;
        GList* uzblinstances;
        
        int fifofd;
        int fb;
        char fifobuf[1024];
        
        void NewTab(char*);
        void SaveSession();
        void LoadSession();
        
        void CloseTab();
        void GotoTab(int);
        void Command(char*);
    
    public:
        void UpdateTablist();
        void CheckFIFO();
    
        UzblTab();
        ~UzblTab();
        
        void Quit();
    
        void Run();
};







#endif /* _UZBLTABEM_H_ */
