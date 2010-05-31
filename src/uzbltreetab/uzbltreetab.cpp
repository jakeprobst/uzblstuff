#include "uzbltreetab.h"

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

void row_clicked(GtkTreeView *view, gpointer data)
{
    UzblTreeTab* ut = (UzblTreeTab*)data;
    
    ut->RowClicked();
}

void cell_edited_callback(GtkCellRendererText *cell, gchar *path, 
                          gchar *text, gpointer data)
{
    UzblTreeTab* ut = (UzblTreeTab*)data;
    
    GtkTreeIter iter;
    gtk_tree_model_get_iter_from_string(GTK_TREE_MODEL(ut->GetTreeStore()), &iter, path);
    
    gtk_tree_store_set(ut->GetTreeStore(), &iter, 0, text, -1);
}
void cell_columns_changed(GtkTreeView* tree, gpointer data)
{
    //GtkTreeModel* model = gtk_tree_view_get_model(tree);
    //int n
    
    for(int i = 0; i < 10; i++)
        printf("===============================\n");
    UzblTreeTab* ut = (UzblTreeTab*)data;
    
    //ut->RowClicked();
    //ut->GotoTab(ut->currenttab);
}

/*void socket_destroyed(GtkSocket *socket, gpointer data)
{
    UzblTreeTab* ut = (UzblTreeTab*)data;
    if (ut->closing)
        return;
    
    int ct = ut->currenttab;    
    
    for(GList* l = ut->uzblinstances; l != NULL; l = g_list_next(l)) {
        UzblInstance* uz = (UzblInstance*)l->data;
        if (socket == uz->GetSocket()) {
            ut->GotoTab(uz->GetNum());
            ut->CloseTab(false);
            ut->GotoTab(ct);
        }
    }
}*/


void wdestroy(GtkWindow* window, UzblTreeTab* ut) {
    ut->Quit();
}
gboolean checkfifo(gpointer data) {
    UzblTreeTab* ut = (UzblTreeTab*)data;
    ut->CheckFIFO();
    
    return true;
}
UzblTreeTab::UzblTreeTab(char* name)
{
    running = true;
    currenttab = 0;
    totaltabs = 0;
    fifocount = 0;
    uzblinstances = NULL;
    fb = 0;
    sessionload = false;
    closing = false;
    
    if (name) {
        sprintf(fifopath, "/tmp/uzbltreetab-%s", name);
        strcpy(tabname, name);
    }
    else {
        sprintf(fifopath, "/tmp/uzbltreetab-%d", rand());
        tabname[0] = '\0';
    }
    mkfifo(fifopath, 0766);
    fifofd = open(fifopath, O_NONBLOCK);
    
    window = GTK_WINDOW(gtk_window_new(GTK_WINDOW_TOPLEVEL));
    char title[1024];
    sprintf(title, "uzbltreetab - %s", name);
    gtk_window_set_title(window, title);
    
    tabtree = GTK_TREE_VIEW(gtk_tree_view_new());
    GtkScrolledWindow* tabscroll = GTK_SCROLLED_WINDOW(gtk_scrolled_window_new(NULL, NULL));
    gtk_scrolled_window_set_policy(tabscroll, GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(tabscroll), GTK_WIDGET(tabtree));
    gtk_widget_set_can_focus(GTK_WIDGET(tabtree), false);
    //gtk_tree_view_set_reorderable(tabtree, true);
    tabmodel = gtk_tree_store_new(1, G_TYPE_STRING);
    gtk_tree_view_set_model(tabtree, GTK_TREE_MODEL(tabmodel));
    
    GtkTreeIter iter;
    GtkCellRenderer* trenderer = gtk_cell_renderer_text_new();
    //g_object_set(trenderer, "editable", true, NULL);
    gtk_tree_view_insert_column_with_attributes(tabtree, -1, "",
                                               trenderer, "text", 0, NULL);
    gtk_tree_view_set_headers_visible(tabtree, false);
    //g_signal_connect(trenderer, "edited", G_CALLBACK(cell_edited_callback), this);
    g_signal_connect(tabtree, "cursor-changed", G_CALLBACK(row_clicked), this);
    g_signal_connect(tabtree, "columns-changed", G_CALLBACK(cell_columns_changed), this);
    
    notebook = GTK_NOTEBOOK(gtk_notebook_new());
    gtk_notebook_set_show_tabs(notebook, false);
    gtk_notebook_set_show_border(notebook, false);

    pane = GTK_PANED(gtk_hpaned_new());
    gtk_paned_add1(pane, GTK_WIDGET(tabscroll));
    gtk_paned_add2(pane, GTK_WIDGET(notebook));
    
    gtk_container_add(GTK_CONTAINER(window), GTK_WIDGET(pane));

    g_signal_connect(window, "destroy", G_CALLBACK(wdestroy), this);
    gtk_widget_show_all(GTK_WIDGET(window));
    gtk_paned_set_position(pane, 200);
    panepos = 200;
    
    //g_timeout_add(1000, updatetabs, this);
    g_timeout_add(100, checkfifo, this);
    
    LoadSession();
    if (totaltabs == 0)
        NewTab("about:blank");
}



UzblTreeTab::~UzblTreeTab()
{
    close(fifofd);
    unlink(fifopath);
    
    for(GList* l = uzblinstances; l != NULL; l = g_list_next(l)) {
        delete ((UzblInstance*)l->data);
    }
    g_list_free(uzblinstances);
}


void UzblTreeTab::SaveSession()
{
    if (sessionload)
        return;
    char spath[FIFOSIZE];
    if (tabname[0])
        sprintf(spath, "%s/uzbl/tabtreesession-%s", getenv("XDG_DATA_HOME"), tabname);
    else
        sprintf(spath, "%s/uzbl/tabtreesession", getenv("XDG_DATA_HOME"));
    
    int sessionfd = open(spath, O_WRONLY|O_CREAT|O_TRUNC);
    fchmod(sessionfd, 0666);
    
    int i = 0;
    for(GList* l = uzblinstances; l != NULL; l = g_list_next(l)) {
        UzblInstance* uzin = (UzblInstance*)l->data;
        
        /*GtkTreeIter iter, parent;
        GtkTreeRowReference* r = uzin->GetRowRef();
        GtkTreePath* p = gtk_tree_row_reference_get_path(r);
        
        gtk_tree_model_get_iter(GTK_TREE_MODEL(tabmodel), &iter, p);
        gtk_tree_model_iter_parent(GTK_TREE_MODEL(tabmodel), &parent, &iter);
        p = gtk_tree_model_get_path(GTK_TREE_MODEL(tabmodel), &parent);*/
        
        sprintf(spath, "%s\t%d\n", uzin->GetURL(), uzin->GetPNum());
        
        write(sessionfd, spath, strlen(spath));
        ++i;
    }
    close(sessionfd);
}


void UzblTreeTab::LoadSession()
{
    char spath[FIFOSIZE];
    if (tabname[0])
        sprintf(spath, "%s/uzbl/tabtreesession-%s", getenv("XDG_DATA_HOME"), tabname);
    else
        sprintf(spath, "%s/uzbl/tabtreesession", getenv("XDG_DATA_HOME"));
    if (access(spath, R_OK|W_OK) == -1)
        return;
 
    printf("p: %s\n", spath);
 
    sessionload = true;
    int sessionfd = open(spath, O_RDWR);
    
    char sb = 0;
    char sbuf[FIFOSIZE];
    memset(sbuf, 0, FIFOSIZE);
    
    while (true) {
        int r = read(sessionfd, sbuf+sb, 1);
        if (r == 0)
            break;
        
        if (sbuf[sb] == '\n') {
            sbuf[sb] = '\0';
            
            if (sbuf[0] != '\0') {
                printf("[%s]\n", sbuf);
                
                char url[FIFOSIZE];
                int child;
                sscanf(sbuf, "%[^\t]\t%d", url, &child);
                printf("[%s:%d]\n", url, child);
                
                NewTab(url, child, false);
                sleep(1);
            }
            
            memset(sbuf, 0, FIFOSIZE);
            sb = -1;
        }
        sb++;
    }
    sessionload = false;
    close(sessionfd);
}


void UzblTreeTab::RebuildTree()
{    
    int i = 0;
    for(GList* l = uzblinstances; l != NULL; l = g_list_next(l)) {
        UzblInstance* cuz = (UzblInstance*)l->data;
        cuz->SetNum(i);
        cuz->SetPNum(-1);
        cuz->ClearChildren();
        cuz->SetRowRef(NULL);
        i++;
    }
    for(GList* l = uzblinstances; l != NULL; l = g_list_next(l)) {
        UzblInstance* cuz = (UzblInstance*)l->data;
        if (cuz->GetParent() != NULL) {
            cuz->SetPNum(cuz->GetParent()->GetNum());
            cuz->GetParent()->AddChild(cuz);
        }
    }
    
    totaltabs = i;
    if (currenttab >= totaltabs)
        currenttab = totaltabs-1;
    if (currenttab < 0)
        currenttab = 0;
    
    gtk_tree_store_clear(tabmodel);
    
    for(GList* l = uzblinstances; l != NULL; l = g_list_next(l)) {
        UzblInstance* iuz = ((UzblInstance*)l->data);
        GtkTreeIter iter;
        if (iuz->GetParent() != NULL) {
            GtkTreeIter piter;
            GtkTreeRowReference* pr = iuz->GetParent()->GetRowRef();
            GtkTreePath* pp = gtk_tree_row_reference_get_path(pr);
            gtk_tree_model_get_iter(GTK_TREE_MODEL(tabmodel), &piter, pp);
            gtk_tree_store_append(tabmodel, &iter, &piter);
        }
        else {
            gtk_tree_store_append(tabmodel, &iter, NULL);
        }
        
        GtkTreePath* p = gtk_tree_model_get_path(GTK_TREE_MODEL(tabmodel), &iter);
        GtkTreeRowReference* ref = gtk_tree_row_reference_new(GTK_TREE_MODEL(tabmodel), p);
        iuz->SetRowRef(ref);
    }
    
    gtk_tree_view_expand_all(tabtree);
    
    printf("%d:%d\n", currenttab, g_list_length(uzblinstances));
    
    GtkTreeIter siter;
    UzblInstance* suz = (UzblInstance*)g_list_nth(uzblinstances, currenttab)->data;
    GtkTreeRowReference* sr = suz->GetRowRef();
    GtkTreePath* sp = gtk_tree_row_reference_get_path(sr);
    gtk_tree_model_get_iter(GTK_TREE_MODEL(tabmodel), &siter, sp);
    
    GtkTreeSelection* sel = gtk_tree_view_get_selection(tabtree);
    gtk_tree_selection_select_iter(sel, &siter);
}

void recurseprint(UzblInstance* uz, int layer)
{
    for(int i = 0; i< layer;++i) {
        printf("  ");
    }
    printf("%s [%s]\n", uz->GetTitle(), uz->GetName());
    
    GList* children = uz->GetChildren();
    if (children != NULL) {
        for(GList* l = children; l != NULL; l = g_list_next(l)) {
            recurseprint((UzblInstance*)l->data, layer+1);
        }
    }
}
void recursedelete(GList* list, UzblInstance* uz)
{
    GList* children = uz->GetChildren();
    if (children) {
        for(GList* l = children; l != NULL; l = g_list_next(l)) {
            recursedelete(list, (UzblInstance*)l->data);
        }
    }
    delete uz;
    list = g_list_remove(list, uz);
}
void UzblTreeTab::CloseTab(UzblInstance* uz, bool closeall)
{
    closing = true;
    
    
    UzblInstance* nuz = NULL;
    UzblInstance* puz = uz->GetParent();
    if (puz) {
        if (g_list_length(puz->GetChildren()) > 1) {
            GList* l = g_list_find(puz->GetChildren(), uz);
            int len = g_list_length(puz->GetChildren())-1;
            printf("%d:%d\n", g_list_position(puz->GetChildren(), l), len);
            if (g_list_position(puz->GetChildren(), l) == len) {
                nuz = ((UzblInstance*)g_list_nth(puz->GetChildren(), len-1)->data);
            }
            else {
                nuz = ((UzblInstance*)g_list_next(l)->data);
            }
        }
        else {
            nuz = puz;
        }
    }
    else {
        if (g_list_last(uzblinstances)->data != uz) {
            for(GList* l = g_list_last(uzblinstances); l != g_list_find(uzblinstances, uz); l = g_list_previous(l)) {
                UzblInstance* luz = (UzblInstance*)l->data;
                if (luz->GetParent() == NULL && luz != uz) {
                    nuz = luz;
                    //break;
                }
            }
        }
        else {
            for(GList* l = g_list_last(uzblinstances); l != g_list_first(uzblinstances); l = g_list_previous(l)) {
                UzblInstance* luz = (UzblInstance*)l->data;
                if (luz->GetParent() == NULL && luz != uz) {
                    nuz = luz;
                    break;
                }
            }
        }
    }
    
    if (closeall) {
        GList* todelete = NULL;
        for(GList* l = uzblinstances; l != NULL; l = g_list_next(l)) {
            UzblInstance* cuz = (UzblInstance*)l->data;
            UzblInstance* cuz2 = cuz;
            
            while (cuz2->GetParent() != uz && cuz2->GetParent() != NULL) {
                cuz2 = cuz2->GetParent();
            }
            if (cuz2->GetParent() != NULL) {
                todelete = g_list_append(todelete, cuz);
            }
        }
        for(GList* l = todelete; l != NULL; l = g_list_next(l)) {
            uzblinstances = g_list_remove(uzblinstances, l->data);
            delete (UzblInstance*)l->data;
        }
        
        uzblinstances = g_list_remove(uzblinstances, uz);
        delete uz;
        g_list_free(todelete);
    }
    else {
        UzblInstance* puz = uz->GetParent();
        GList* children = uz->GetChildren();
        for(GList* l = children; l != NULL; l = g_list_next(l)) {
            UzblInstance* cuz = (UzblInstance*)l->data;

            cuz->SetParent(puz);
        }
        uzblinstances = g_list_remove(uzblinstances, uz);
        delete uz;
    }
    
    RebuildTree();
    UpdateTablist();
    
    if (nuz)
        GotoTab(nuz->GetNum());
    else
        GotoTab(gtk_notebook_get_current_page(notebook));
    SaveSession();
    closing = false;
}


void UzblTreeTab::GotoTab(int i)
{
    currenttab = i;
    
    if (currenttab < 0)
        currenttab = totaltabs -1;
    if (currenttab >= totaltabs)
        currenttab = 0;
    
    GtkTreeIter iter;
    UzblInstance* uz = (UzblInstance*)g_list_nth(uzblinstances, currenttab)->data;
    GtkTreeRowReference* r = uz->GetRowRef();
    GtkTreePath* p = gtk_tree_row_reference_get_path(r);
    gtk_tree_model_get_iter(GTK_TREE_MODEL(tabmodel), &iter, p);
    
    int page = gtk_notebook_page_num(notebook, GTK_WIDGET(uz->GetSocket()));
    if (page == -1) {
        CloseTab(uz, false);
        return;
    }
    
    gtk_notebook_set_current_page(notebook, page);
    GtkTreeSelection* sel = gtk_tree_view_get_selection(tabtree);
    gtk_tree_selection_select_iter(sel, &iter);
}


void UzblTreeTab::Command(char* c)
{
    char** cmd = g_strsplit(c, " ", -1);
    do {
    if (!g_strcmp0(cmd[0], "new")) {
        if (cmd[1]) {
            char* u = g_strjoinv(" ",cmd+1);
            NewTab(u);
            g_free(u);
        }
        else 
            NewTab("about:blank");
        SaveSession();
    }
    if (!g_strcmp0(cmd[0], "cnew")) {
        if (cmd[2]) {
            int i = 0;
            for(GList* l = uzblinstances; l != NULL; l = g_list_next(l), i++) {
                UzblInstance* uzin = (UzblInstance*)l->data;
                printf("g_strcmp0(%s, %s), %d\n", uzin->GetName(), cmd[1], !g_strcmp0(uzin->GetName(), cmd[1]));
                if (!g_strcmp0(uzin->GetName(), cmd[1])) {
                    char* u = g_strjoinv(" ",cmd+2);
                    NewTab(u, i);
                    g_free(u);
                    break;
                }
            }
        }
        else 
            NewTab("about:blank", currenttab);
        SaveSession();
    }
    if (!g_strcmp0(cmd[0], "close")) {
        for(GList* l = uzblinstances; l != NULL; l = g_list_next(l)) {
            UzblInstance* uzin = (UzblInstance*)l->data;
            if (!g_strcmp0(uzin->GetName(), cmd[1])) {
                printf("closing: %s\n", uzin->GetName());
                CloseTab(uzin, false);
                break;
            }
        }
    }
    if (!g_strcmp0(cmd[0], "treeclose")) {
        for(GList* l = uzblinstances; l != NULL; l = g_list_next(l)) {
            UzblInstance* uzin = (UzblInstance*)l->data;
            if (!g_strcmp0(uzin->GetName(), cmd[1])) {
                printf("treeclosing: %s\n", uzin->GetName());
                CloseTab(uzin, true);
                break;
            }
        }
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
    if (!g_strcmp0(cmd[0], "attach")) {
        int a;
        if (cmd[1]) {
            sscanf(cmd[1], "%d", &a);
            if (a > totaltabs)
                break;
            
            UzblInstance* cuz = (UzblInstance*)g_list_nth(uzblinstances, currenttab)->data;
            UzblInstance* puz = (UzblInstance*)g_list_nth(uzblinstances, a)->data;
            
            if (cuz == puz)
                break;
            
            if (cuz->GetNum() < puz->GetNum()) {
                uzblinstances = g_list_remove(uzblinstances, cuz);
                uzblinstances = g_list_insert(uzblinstances, cuz, puz->GetNum()+1);
            }
            
            cuz->SetParent(puz);
            RebuildTree();
            UpdateTablist();
            SaveSession();
        }
    }
    if (!g_strcmp0(cmd[0], "move")) {
        int a;
        if (cmd[1]) {
            sscanf(cmd[1], "%d", &a);
            UzblInstance* cuz = (UzblInstance*)g_list_nth(uzblinstances, currenttab)->data;
            uzblinstances = g_list_remove(uzblinstances, cuz);
            
            if (a > totaltabs)
                a = totaltabs;
            
            uzblinstances = g_list_insert(uzblinstances, cuz, a);
            currenttab = a;
            RebuildTree();
            UpdateTablist();
            SaveSession();
        }
    }
    if (!g_strcmp0(cmd[0], "tabtitle")) {
        for(GList* l = uzblinstances; l != NULL; l = g_list_next(l)) {
           if (!strcmp(cmd[1], ((UzblInstance*)l->data)->GetName())) {
               char* title = g_strjoinv(" ", cmd+2);
               ((UzblInstance*)l->data)->SetTitle(title);
               g_free(title);
           }
        }
        UpdateTablist();
    }
    if (!g_strcmp0(cmd[0], "taburi")) {
        for(GList* l = uzblinstances; l != NULL; l = g_list_next(l)) {
           if (!strcmp(cmd[1], ((UzblInstance*)l->data)->GetName())) {
               char* url = g_strjoinv(" ", cmd+2);
               ((UzblInstance*)l->data)->SetURL(url);
               g_free(url);
               SaveSession();
           }
        }
    }
    /*if (!g_strcmp0(cmd[0], "tabclose")) { // more like crash
        for(GList* l = uzblinstances; l != NULL; l = g_list_next(l)) {
           if (!strcmp(cmd[1], ((UzblInstance*)l->data)->GetSocketPath())) {
               printf("s: %s\n", ((UzblInstance*)l->data)->GetURL());
               GotoTab(((UzblInstance*)l->data)->GetNum());
               CloseTab(false);
               SaveSession();
           }
        }
    }*/
    if (!g_strcmp0(cmd[0], "showtree")) {
        gtk_paned_set_position(pane, panepos);
    }
    if (!g_strcmp0(cmd[0], "hidetree")) {
        panepos = gtk_paned_get_position(pane);
        gtk_paned_set_position(pane, 0);
    }
    } while(0);
    g_strfreev(cmd);
}

void UzblTreeTab::UpdateTablist()
{
    int i = 0;
    
    for(GList* l = uzblinstances; l != NULL; l = g_list_next(l)) {
        GtkTreeIter iter;
        char str[1024];
        memset(str, 0, 1024);
        
        //UzblInstance* uz = (UzblInstance*)g_list_nth(uzblinstances, i)->data;
        UzblInstance* uz = (UzblInstance*)l->data;
        //uz->SetNum(i);
        
        GtkTreeRowReference* r = uz->GetRowRef();
        GtkTreePath* p = gtk_tree_row_reference_get_path(r);
        gtk_tree_model_get_iter(GTK_TREE_MODEL(tabmodel), &iter, p);
        
        sprintf(str, "%d: %s", i++, uz->GetTitle());
        gtk_tree_store_set(GTK_TREE_STORE(tabmodel), &iter,
                       0, str, -1);
    }
}

void UzblTreeTab::RowClicked()
{
    GtkTreeIter iter;
    GtkTreeSelection* sel = gtk_tree_view_get_selection(tabtree);
    gtk_tree_selection_get_selected(sel, NULL, &iter);
    GtkTreePath* path = gtk_tree_model_get_path(GTK_TREE_MODEL(tabmodel), &iter);
    
    int i = 0;
    for(GList* l = uzblinstances; l != NULL; l = g_list_next(l)) {
        UzblInstance* uz = (UzblInstance*)l->data;
        
        GtkTreeRowReference* r = uz->GetRowRef();
        GtkTreePath* p = gtk_tree_row_reference_get_path(r);

        if (!gtk_tree_path_compare(path, p)) {
            GotoTab(i);
            break;
        }
        i++;
    }
}



void UzblTreeTab::CheckFIFO()
{
    while (true) {
        int r = read(fifofd, fifobuf+fb, 1);
        if (r == 0)
            return;
        
        if (fifobuf[fb] == '\n') {
            fifobuf[fb] = '\0';
            Command(fifobuf);
            printf("fifobuf: [%s]\n", fifobuf);
            memset(fifobuf, 0, FIFOSIZE);
            fb = -1;
        }
        
        fb++;
    }
}

void UzblTreeTab::NewTab(char* url, int child, bool save)
{
    /*char sockid[128];
    sprintf(sockid, "%d_%d", getpid(), fifocount++);*/

    GtkTreeIter iter;
    if (child != -1 && child < g_list_length(uzblinstances)) {
        GtkTreeRowReference* r = ((UzblInstance*)g_list_nth(uzblinstances, child)->data)->GetRowRef();
        
        GtkTreePath* p = gtk_tree_row_reference_get_path(r);
        
        GtkTreeIter piter;
        gtk_tree_model_get_iter(GTK_TREE_MODEL(tabmodel), &piter, p);
        
        gtk_tree_store_append(GTK_TREE_STORE(tabmodel), &iter, &piter);
        
        gtk_tree_view_expand_row(tabtree, p, false);
    }
    else
        gtk_tree_store_append(GTK_TREE_STORE(tabmodel), &iter, NULL);
    
    GtkTreePath* p = gtk_tree_model_get_path(GTK_TREE_MODEL(tabmodel), &iter);

    GtkTreeRowReference* ref = gtk_tree_row_reference_new(GTK_TREE_MODEL(tabmodel), p);
    
    UzblInstance* uzin = new UzblInstance(url, fifopath, notebook, ref);
    uzin->SetPNum(-1);
    uzin->SetParent(NULL);
    
    
    GtkTreeIter piter;
    GtkTreePath* parent = NULL;
    if(gtk_tree_model_iter_parent(GTK_TREE_MODEL(tabmodel), &piter, &iter))
        parent = gtk_tree_model_get_path(GTK_TREE_MODEL(tabmodel), &piter);
    
    
    int i = 0;
    for(GList* l = uzblinstances; l != NULL; l = g_list_next(l)) {
        UzblInstance* uz = (UzblInstance*)l->data;
        
        GtkTreeRowReference* r = uz->GetRowRef();
        GtkTreePath* p2 = gtk_tree_row_reference_get_path(r);
        
        if (parent == NULL)
            break;

        if (!gtk_tree_path_compare(p2, parent)) {
            uzin->SetPNum(i);
            uzin->SetParent(uz);
            uz->AddChild(uzin);
            break;
        }
        i++;
    }
    
    uzblinstances = g_list_append(uzblinstances, uzin);
    totaltabs++;
    uzin->SetNum(totaltabs);
    
    //g_signal_connect(uzin->GetSocket(), "destroy", G_CALLBACK(socket_destroyed), this);
    
    //gtk_notebook_set_current_page(notebook, gtk_notebook_get_current_page(notebook));
    
    UzblInstance* tuz = (UzblInstance*)g_list_nth(uzblinstances, currenttab)->data;
    gtk_widget_grab_focus(GTK_WIDGET(tuz->GetSocket()));
    
    if (save)
        SaveSession();
    UpdateTablist();
    GotoTab(currenttab);
}


void UzblTreeTab::Quit()
{
    running = false;
    SaveSession();
    gtk_main_quit();
}


void UzblTreeTab::Run()
{
    gtk_main();
}


