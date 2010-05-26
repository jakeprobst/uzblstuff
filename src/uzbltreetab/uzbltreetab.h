#ifndef _UZBLTABEM_H_
#define _UZBLTABEM_H_

#include <gtk/gtk.h>
#include <stdlib.h>

#include "uzblinstance.h"

const int FIFOSIZE = 1024*4;

class UzblTreeTab {
    private:
        bool running;
        
        int totaltabs;
        int fifocount;
        
        GtkWindow* window;
        GtkNotebook* notebook;
        GtkTreeView* tabtree;
        GtkTreeStore* tabmodel;
        
        int panepos;
        GtkPaned* pane;
        
        char tabname[256];
        char fifopath[256];
        bool sessionload;
        int fifofd;
        int fb;
        char fifobuf[FIFOSIZE];
        
        void RebuildTree();
        
        void NewTab(char*, int child = -1, bool save = true);
        //void NewTab2(char*, char*);
        void SaveSession();
        void LoadSession();
        
        //void CloseTabRecurse(UzblInstance*);
        
        void Command(char*);
    
    public:
        int currenttab;
        GList* uzblinstances;
        bool closing;
    
        void CloseTab(UzblInstance*, bool);
        void GotoTab(int);
        void CheckFIFO();
        void UpdateTablist();
        void RowClicked();
        
        GtkTreeStore* GetTreeStore() {return tabmodel;};
    
        UzblTreeTab(char* a = NULL);
        ~UzblTreeTab();
        
        void Quit();
    
        void Run();
};







#endif /* _UZBLTABEM_H_ */
