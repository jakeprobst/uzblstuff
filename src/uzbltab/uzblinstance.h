#ifndef _UZBLINSTANCE_H_
#define _UZBLINSTANCE_H_

#include <gtk/gtk.h>


class UzblInstance {
    private:
        GtkSocket* socket;
        GPid pid;
        char* url;
        char* title;
        
        char fifopath[256];
        char sockpath[256];
    
    public:
        void SetURL(char*);
        char* GetURL() {return url;};
        void SetTitle(char*);
        char* GetTitle() {return title;};
        char* GetFifoPath() {return fifopath;}
        char* GetSocketPath() {return sockpath;}
        
        GtkSocket* GetSocket() {return socket;};

        UzblInstance(char*, char*, GtkNotebook*);
        ~UzblInstance();
};



















#endif /* _UZBLINSTANCE_H_ */
