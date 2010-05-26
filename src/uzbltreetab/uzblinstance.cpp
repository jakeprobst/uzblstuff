#include "uzblinstance.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

//int UzblInstance::ID = 0;

UzblInstance::UzblInstance(char* u, char* mainfifo, GtkNotebook* n, GtkTreeRowReference* r)
{
    //id = ID++;
    url = g_strdup(u);
    title = g_strdup("...");
        
    /*sprintf(fifopath, "/tmp/uzbl_fifo_%s", fifo);
    sprintf(sockpath, "/tmp/uzbl_socket_%s", fifo);*/
    
    row = r;
    notebook = n;
    children = NULL;
    
    socket = GTK_SOCKET(gtk_socket_new());
    gtk_widget_set_can_focus(GTK_WIDGET(socket), false);
    gtk_notebook_append_page(notebook, GTK_WIDGET(socket), NULL);
    gtk_widget_show_all(GTK_WIDGET(notebook));

    int rn;
    FILE* f = fopen("/dev/urandom", "rb");
    fread(&rn, sizeof(int), 1, f);
    fclose(f);
    sprintf(name, "%d", rn &0x7FFFFFFF);

    char cmd[1024];
    sprintf(cmd, "uzbl-browser tab %d %s %s \"%s\"",
                 gtk_socket_get_id(socket), name, mainfifo, u);
    
    printf("%s\n", cmd);
    g_spawn_command_line_async(cmd, NULL);
}

UzblInstance::~UzblInstance()
{
    g_free(url);
    g_free(title);
    
    /*unlink(fifopath);
    unlink(sockpath);
    g_spawn_close_pid(pid);*/
    
    if (gtk_notebook_page_num(notebook, GTK_WIDGET(socket)) != -1)
        gtk_notebook_remove_page(notebook, gtk_notebook_page_num(notebook, GTK_WIDGET(socket)));
}

void UzblInstance::SetURL(char* u)
{
    if (url)
        g_free(url);
    url = g_strdup(u);
}

void UzblInstance::SetTitle(char* t)
{
    if (title)
        g_free(title);
    title = g_strdup(t);
}





