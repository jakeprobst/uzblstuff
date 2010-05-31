#ifndef _UZBLINSTANCE_H_
#define _UZBLINSTANCE_H_

#include <gtk/gtk.h>


class UzblInstance {
    //static int ID;
    private:
        //int id;
        GtkSocket* socket;
        GtkNotebook* notebook;
        GtkTreeRowReference* row;
        //GPid pid;
        
        char* url;
        char* title;
        
        UzblInstance* parent;
        GList* children;
        
        //int name;
        int num;
        int pnum;
        
        char name[16];
        
        //char fifopath[256];
        //char sockpath[256];
    
    public:
        void SetURL(char*);
        char* GetURL() {return url;};
        void SetTitle(char*);
        char* GetTitle() {return title;};
        //char* GetFifoPath() {return fifopath;}
        //char* GetSocketPath() {return sockpath;}
        char* GetName() {return name;};
        void SetNum(int n) {num = n;};
        int GetNum() {return num;};
        void SetPNum(int n) {pnum = n;};
        int GetPNum() {return pnum;};
        void SetParent(UzblInstance* p) {parent = p;};
        UzblInstance* GetParent() {return parent;};
        void AddChild(UzblInstance* c) {children = g_list_append(children, c);};
        GList* GetChildren() {return children;};
        void ClearChildren() {g_list_free(children); children=NULL;};
        //int GetID() {return id;};
        
        void SetRowRef(GtkTreeRowReference* r) {if (row) {gtk_tree_row_reference_free(row);}; row = r;};
        GtkTreeRowReference* GetRowRef() {return row;};
        
        GtkSocket* GetSocket() {return socket;};

        UzblInstance(const char*, const char*, GtkNotebook*, GtkTreeRowReference*);
        ~UzblInstance();
};



















#endif /* _UZBLINSTANCE_H_ */
