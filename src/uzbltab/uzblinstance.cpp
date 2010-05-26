#include "uzblinstance.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

UzblInstance::UzblInstance(char* u, char* fifo, GtkNotebook* notebook)
{
    url = g_strdup(u);
    title = g_strdup("Loading...");
        
    sprintf(fifopath, "/tmp/uzbl_fifo_%s", fifo);
    sprintf(sockpath, "/tmp/uzbl_socket_%s", fifo);
    
    socket = GTK_SOCKET(gtk_socket_new());
    gtk_notebook_append_page(notebook, GTK_WIDGET(socket), NULL);
    gtk_widget_show_all(GTK_WIDGET(notebook));

    char cmd[1024];
    
    sprintf(cmd, "uzbl-browser tab %d %s \"%s\"",
                 gtk_socket_get_id(socket), sockpath, u);
    
    printf("%s\n", cmd);
    g_spawn_command_line_async(cmd, NULL);
}

UzblInstance::~UzblInstance()
{
    //gtk_widget_destroy(GTK_WIDGET(socket));
    g_object_ref_sink(G_OBJECT(socket));
    gtk_object_unref(GTK_OBJECT(socket));
    g_free(url);
    g_free(title);
    
    unlink(fifopath);
    unlink(sockpath);
    g_spawn_close_pid(pid);
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





