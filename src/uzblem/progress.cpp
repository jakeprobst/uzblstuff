#include "progress.h"
#include "uzblem.h"

#include <stdlib.h>

Progress::Progress(UzblEM* e)
{
    em = e;
    
    width = 8,
    done = strdup("=");
    pending = strdup(" ");
    format = strdup("[%d%a%p]%c");
    spinner = strdup("-\\|/");
    updates = 0;
    progress = 100;
}


Progress::~Progress()
{
    delete[] done;
    delete[] pending;
    delete[] format;
    delete[] spinner;
}


void Progress::ProgressConfig(char** cmd)
{
    //char* c = strjoin(" ", cmd+3);
    //printf("pconf: %s:%s\n", cmd[1], c);
    char* c = cmd[3];
    
    if (!strcmp(cmd[1], "width"))
        width = atoi(c);
    if (!strcmp(cmd[1], "updates"))
        updates = atoi(c);
    if (!strcmp(cmd[1], "progress"))
        progress = atoi(c);
    if (!strcmp(cmd[1], "done")) {
        delete[] done;
        done = strdup(c);
    }
    if (!strcmp(cmd[1], "pending")) {
        delete[] pending;
        pending = strdup(c);
    }
    if (!strcmp(cmd[1], "format")) {
        delete[] format;
        format = strdup(c);
    }
    if (!strcmp(cmd[1], "spinner")) {
        delete[] spinner;
        spinner = strdup(c);
    }
    
    //delete[] c;
}

void Progress::ResetProgress()
{
    updates = progress = 0;
}


void Progress::UpdateProgress(char** cmd)
{
    int prog = atoi(cmd[1]);
    
    int d = int(((prog/100.0)*width)+0.5);
    int p = width - d;
    
    char out[1024];
    memset(out, 0, 1024);
    strcpy(out, format);
    //char* out;
    if (strcontains(out, "%d")) {
        char w[(width*strlen(done))+2];
        memset(w, 0, (width*strlen(done))+2);
        for(int i = 0; i < d; i++)
            strcat(w, done);
        char* o = strreplace(out, "%d", w, -1);
        strcpy(out, o);
        delete[] o;
    }
    if (strcontains(out, "%p")) {
        char w[(p*strlen(pending))+2];
        memset(w, 0, (p*strlen(pending))+2);
        for(int i = 0; i < p; i++)
            strcat(w, pending);
        char* o = strreplace(out, "%p", w, -1);
        strcpy(out, o);
        delete[] o;
    }
    if (strcontains(out, "%c")) {
        char w[32];
        sprintf(w, "%d%%", prog);
        char* o = strreplace(out, "%c", w, -1);
        strcpy(out, o);
        delete[] o;
    }
    if (strcontains(out, "%i")) {
        char w[32];
        sprintf(w, "%d", prog);
        char* o = strreplace(out, "%i", w, -1);
        strcpy(out, o);
        delete[] o;
    }
    if (strcontains(out, "%t")) {
        char w[32];
        sprintf(w, "%d%%", 100-prog);
        char* o = strreplace(out, "%t", w, -1);
        strcpy(out, o);
        delete[] o;
    }
    if (strcontains(out, "%o")) {
        char w[32];
        sprintf(w, "%d", 100-prog);
        char* o = strreplace(out, "%o", w, -1);
        strcpy(out, o);
        delete[] o;
    }
    if (strcontains(out, "%s")) {
        char w[4];
        memset(w, 0, 4);
        w[0] = spinner[updates % strlen(spinner)];
        if (w[0] == '\\')
            w[1] = '\\';
        //printf("w: %s\n", w);
        char* o = strreplace(out, "%s", w, -1);
        strcpy(out, o);
        delete[] o;
    }
    
    
    char sp[1024];
    sprintf(sp, "set progress_format = %s", out);
    em->SendCommand(sp);
    
    updates++;
}


void Progress::Event(char** cmd)
{
    if (!strcmp(cmd[0], "PROGRESS_CONFIG"))
        ProgressConfig(cmd);
    if (!strcmp(cmd[0], "LOAD_COMMIT"))
        ResetProgress();
    if (!strcmp(cmd[0], "LOAD_PROGRESS"))
        UpdateProgress(cmd);
}


























