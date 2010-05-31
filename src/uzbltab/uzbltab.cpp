#include "uzbltab.h"

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

const char* TAB_COLOR = "foreground = \"#888\"";
const char* TAB_TEXT_COLOR = "foreground = \"#bbb\"";
const char* SELECTED_TAB_COLOR = "foreground = \"#fff\"";
const char* SELECTED_TAB_TEXT_COLOR = "foreground = \"green\"";



char* strreplace(char* str, const char* replaceme, const char* replaceto, int times)
{
    int sofar = 0;
    int occurances = 0;
    int slen = strlen(str);
    int rmlen = strlen(replaceme);
    
    for(int i = 0; i < slen-rmlen+1; i++) {
        if (!strncmp(str+i, replaceme, rmlen)) {
            occurances++;
        }
    }
    
    int olen = strlen(str) + strlen(replaceto)*occurances + 1;
    char* out = new char[olen];
    memset(out, 0, olen);
    
    int index = 0;
    for(int i = 0; i < slen; i++) {
        if (!strncmp(str+i, replaceme, rmlen)) {
            if (sofar >= times && times != -1)
                strncat(out, replaceme, olen);
            else
                strncat(out, replaceto, olen);
            index = strlen(out);
            i += strlen(replaceme) -1;
            sofar++;
            
        }
        else {
            out[index++] = str[i];
        }
    }
    
    return out;
}


gchar* escape_string(gchar* s)
{
    if (s == NULL || strlen(s) == 0)
        return NULL;
    gchar* n = (gchar*)g_malloc0(strlen(s)*2);
    gint i, b=0;
    for(i = 0; i < strlen(s); i++) {
        if (s[i] == '&') {
            n[b++] = '&';
            n[b++] = 'a';
            n[b++] = 'm';
            n[b++] = 'p';
            n[b++] = ';';
            continue;
        }
        if (s[i] == '>') {
            n[b++] = '&';
            n[b++] = 'g';
            n[b++] = 't';
            n[b++] = ';';
            continue;
        }
        if (s[i] == '<') {
            n[b++] = '&';
            n[b++] = 'l';
            n[b++] = 't';
            n[b++] = ';';
            continue;
        }
        
        n[b++] = s[i];
    }
    n[b++] = '\0';
    
    return n;
}


void wdestroy(GtkWindow* window, UzblTab* ut) {
    ut->Quit();
}
gboolean updatetabs(gpointer data) {
    UzblTab* ut = (UzblTab*)data;
    ut->UpdateTablist();
    
    return true;
}
gboolean checkfifo(gpointer data) {
    UzblTab* ut = (UzblTab*)data;
    ut->CheckFIFO();
    
    return true;
}
UzblTab::UzblTab()
{
    running = true;
    currenttab = 0;
    totaltabs = 0;
    fifocount = 0;
    uzblinstances = NULL;
    fb = 0;
    
    mkfifo("/tmp/uzbltab", 0766);
    fifofd = open("/tmp/uzbltab", O_NONBLOCK);
    
    window = GTK_WINDOW(gtk_window_new(GTK_WINDOW_TOPLEVEL));
    
    tablist = GTK_LABEL(gtk_label_new("[0: ]"));
    gtk_label_set_use_markup(tablist, true);
    gtk_label_set_justify(tablist, GTK_JUSTIFY_LEFT);
    gtk_label_set_line_wrap(tablist, false);
    gtk_label_set_selectable(tablist, false);
    gtk_misc_set_padding(GTK_MISC(tablist), 2, 2);
    GtkEventBox* ebox = GTK_EVENT_BOX(gtk_event_box_new());
    GtkAlignment* align = GTK_ALIGNMENT(gtk_alignment_new(0.0f, 0.0f, 0.0f, 0.0f));
    gtk_container_add(GTK_CONTAINER(align), GTK_WIDGET(tablist));
    gtk_container_add(GTK_CONTAINER(ebox), GTK_WIDGET(align));

    GdkColor color; 
    gdk_color_parse("#303030", &color);
    
    gtk_widget_modify_bg(GTK_WIDGET(ebox), GTK_STATE_NORMAL, &color);

    notebook = GTK_NOTEBOOK(gtk_notebook_new());
    gtk_notebook_set_show_tabs(notebook, false);
    gtk_notebook_set_show_border(notebook, false);
    
    GtkBox* box = GTK_BOX(gtk_vbox_new(false, 0));
    gtk_box_pack_start(box, GTK_WIDGET(ebox), false, false, 0);
    gtk_box_pack_start(box, GTK_WIDGET(notebook), true, true, 0);
    
    gtk_container_add(GTK_CONTAINER(window), GTK_WIDGET(box));

    g_signal_connect(window, "destroy", G_CALLBACK(wdestroy), this);
    gtk_widget_show_all(GTK_WIDGET(window));
    
    g_timeout_add(1000, updatetabs, this);
    g_timeout_add(100, checkfifo, this);
    
    LoadSession();
    if (totaltabs == 0)
        NewTab("google.com");
}



UzblTab::~UzblTab()
{
    close(fifofd);
    
    for(GList* l = uzblinstances; l != NULL; l = g_list_next(l)) {
        delete ((UzblInstance*)l->data);
    }
    g_list_free(uzblinstances);
}


void UzblTab::SaveSession()
{
    char spath[1024];
    sprintf(spath, "%s/uzbl/session", getenv("XDG_DATA_HOME"));
    
    int sessionfd = open(spath, O_WRONLY|O_TRUNC);
    
    int i = 0;
    for(GList* l = uzblinstances; i < totaltabs; l = g_list_next(l)) {
        UzblInstance* uzin = (UzblInstance*)l->data;
        printf("w: [%s]\n", uzin->GetURL());
        write(sessionfd, uzin->GetURL(), strlen(uzin->GetURL()));
        write(sessionfd, "\n", 1);
        ++i;
    }
    close(sessionfd);
}


void UzblTab::LoadSession()
{
    char spath[1024];
    sprintf(spath, "%s/uzbl/session", getenv("XDG_DATA_HOME"));
    if (access(spath, R_OK|W_OK) == -1)
        return;
    
    int sessionfd = open(spath, O_RDWR);
    
    char sb = 0;
    char sbuf[1024];
    memset(sbuf, 0, 1024);
    
    while (true) {
        int r = read(sessionfd, sbuf+sb, 1);
        if (r == 0)
            return;
        
        if (sbuf[sb] == '\n') {
            sbuf[sb] = '\0';
            
            if (sbuf[0] != '\0')
                NewTab(sbuf);
            
            memset(sbuf, 0, 1024);
            sb = -1;
        }
        sb++;
    }
}


void UzblTab::UpdateTablist()
{
    char tab_format[] = "<span %s> [ %d <span %s>%.50s</span> ] </span>";
    
    // safely assuming no one will be able to fill all that
    char str[8*1024];
    char oldstrline[2*1024];
    char strline[2*1024];
    memset(str, 0, 8*1024);
    memset(strline, 0, 2*1024);
        
    int winwidth, winheight;
    gtk_window_get_size(window, &winwidth, &winheight);
    
    int i = 0;
    for(GList* l = uzblinstances; l != NULL; l = g_list_next(l), ++i) {
        char item[1024];
        
        UzblInstance* uzin = (UzblInstance*)l->data;
        
        char* tabfg;
        char* txtfg;
        if (currenttab == i) {
            tabfg = g_strdup(SELECTED_TAB_COLOR);
            txtfg = g_strdup(SELECTED_TAB_TEXT_COLOR);
        }
        else {
            tabfg = g_strdup(TAB_COLOR);
            txtfg = g_strdup(TAB_TEXT_COLOR);
        }
        char* escstr = escape_string(uzin->GetTitle());
        char* escstrr = strreplace(escstr, "&", "&amp;", -1);
        sprintf(item, tab_format, tabfg, i, txtfg, escstrr);
        delete[] escstrr;
        
        g_strlcpy(oldstrline, strline, 2*1024);
        g_strlcat(strline, item, 2*1024);
        
        GtkLabel* label = GTK_LABEL(gtk_label_new(strline));
        if (label == NULL) {
            printf("label broke somewhere!\n");
        }
        else {
            gtk_label_set_use_markup(label, true);
            
            gtk_widget_show(GTK_WIDGET(label));
            
            int lwidth;
            PangoLayout* layout = gtk_label_get_layout(label);
            pango_layout_get_pixel_size(layout, &lwidth, &winheight);
            
            if (lwidth > (winwidth - 20)) {
                g_strlcat(str, oldstrline, 8*1024);
                g_strlcat(str, "&#10;", 8*1024);
                memset(strline, 0, 2*1024);
                g_strlcat(strline, item, 2*1024);
            }
            
            //gtk_widget_destroy(GTK_WIDGET(label));
            g_object_ref_sink(G_OBJECT(label));
            gtk_object_unref(GTK_OBJECT(label));
            
            g_free(escstr);
        }
        
        g_free(tabfg);
        g_free(txtfg);
    }
    
    g_strlcat(str, strline, 8*1024);
    gtk_label_set_markup(tablist, str);
}



void UzblTab::CloseTab()
{
    gtk_notebook_remove_page(notebook, currenttab);
        
    GList* l = g_list_nth(uzblinstances, currenttab);
    delete (UzblInstance*)l->data;
    l->data = NULL;
    uzblinstances = g_list_remove_link(uzblinstances, l);
    
    totaltabs--;

    if (currenttab >= totaltabs)
        currenttab = totaltabs-1;

    UpdateTablist();
    SaveSession();
}


void UzblTab::GotoTab(int i)
{
    currenttab = i;
    
    if (currenttab < 0)
        currenttab = totaltabs -1;
    if (currenttab >= totaltabs)
        currenttab = 0;
    
    gtk_notebook_set_current_page(notebook, currenttab);
    UpdateTablist();
}


void UzblTab::Command(char* c)
{
    char** cmd = g_strsplit(c, " ", -1);
    if (!g_strcmp0(cmd[0], "new")) {
        if (cmd[1])
            NewTab(cmd[1]);
        else 
            NewTab("about:blank");
        SaveSession();
    }
    if (!g_strcmp0(cmd[0], "close")) {
        CloseTab();
    }
    if (!g_strcmp0(cmd[0], "next")) {
        GotoTab(currenttab+1);
    }
    if (!g_strcmp0(cmd[0], "prev")) {
        GotoTab(currenttab-1);
    }
    if (!g_strcmp0(cmd[0], "first")) {
        GotoTab(0);
    }
    if (!g_strcmp0(cmd[0], "last")) {
        GotoTab(totaltabs-1);
    }
    if (!g_strcmp0(cmd[0], "goto")) {
        int a;
        if (cmd[1]) {
            sscanf(cmd[1], "%d", &a);
            GotoTab(a);
        }
    }
    if (!g_strcmp0(cmd[0], "tabtitle")) {
        for(GList* l = uzblinstances; l != NULL; l = g_list_next(l)) {
           if (!strcmp(cmd[1], ((UzblInstance*)l->data)->GetSocketPath())) {
               char* title = g_strjoinv(" ", cmd+2);
               ((UzblInstance*)l->data)->SetTitle(title);
               g_free(title);
           }
        }
    }
    if (!g_strcmp0(cmd[0], "taburi")) {
        for(GList* l = uzblinstances; l != NULL; l = g_list_next(l)) {
           if (!strcmp(cmd[1], ((UzblInstance*)l->data)->GetSocketPath())) {
               char* url = g_strjoinv(" ", cmd+2);
               ((UzblInstance*)l->data)->SetURL(url);
               g_free(url);
           }
        }
    }
    
    g_strfreev(cmd);
}


void UzblTab::CheckFIFO()
{
    while (true) {
        int r = read(fifofd, fifobuf+fb, 1);
        if (r == 0)
            return;
        
        if (fifobuf[fb] == '\n') {
            fifobuf[fb] = '\0';
            Command(fifobuf);
            memset(fifobuf, 0, 1024);
            fb = -1;
        }
        
        fb++;
    }
}

void UzblTab::NewTab(const char* url)
{
    char sockid[128];
    sprintf(sockid, "%d_%d", getpid(), fifocount++);
        
    UzblInstance* uzin = new UzblInstance(url, sockid, notebook);
    
    uzblinstances = g_list_append(uzblinstances, uzin);
    totaltabs++;
    
    UpdateTablist();
}


void UzblTab::Quit()
{
    running = false;
    SaveSession();
    gtk_main_quit();
    
}


void UzblTab::Run()
{
    
    gtk_main();
}


