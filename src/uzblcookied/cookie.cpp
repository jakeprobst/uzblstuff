#include <time.h>
#include <stdlib.h>

#include "cookie.h"
#include "util.h"

const char* STRPTIME = "%a, %d-%b-%Y %H:%M:%S %Z";
//Fri, 22-May-2020 03:25:25 GMT

bool cookiesort(const Cookie& a, const Cookie& b)
{
    if (strcmp(a.domain, b.domain) < 0)
        return true;
    if (strcmp(a.path, b.path) < 0)
        return true;
    return false;
}

Cookie::Cookie(const Cookie& other)
{
    expires = other.expires;
    secure = other.secure;
    domain = strdup(other.domain);
    path = strdup(other.path);
    key = strdup(other.key);
    value = strdup(other.value);
}

Cookie::Cookie(char* data)
{
    char tdomain[512];
    char tbleh[32];
    char tpath[512];
    char tsecure[32];
    char texpires[32];
    char tkey[1024*4];
    char tvalue[1024*4];
    
    memset(tvalue, 0, 1024*4);
    sscanf(data, "%[^\t]\t%[^\t]\t%[^\t]\t%[^\t]\t%[^\t]\t%[^\t]\t%[^\t]", 
                        tdomain, tbleh, tpath, tsecure, texpires, tkey, tvalue);
                        
    // sscanf ignores the expire slot if it is empty and messes everything up
    if (strlen(tvalue) == 0) {
        strcpy(tvalue, tkey);
        strcpy(tkey, texpires);
        memset(texpires, 0, 32);
        texpires[0] = '0'; // zero, not null
    }
    
    domain = strdup(tdomain);
    path = strdup(tpath);
    secure = false;
    if (!strcmp(tsecure, "TRUE"))
        secure = true;
    expires = 0;
    if (strlen(texpires))
        sscanf(texpires, "%d", &expires);
    key = strdup(tkey);
    value = strdup(tvalue);
}

Cookie::Cookie(char* host, char* data)
{
    domain = strdup(host);
    expires = 0;
    secure = false;
    path = NULL;
    key = strdup("");
    value = strdup("");

    char** vars = strsplit(data, ';');
    for(int i = 0; vars[i]; i++) {
        int o = 0;
        if (vars[i][0] == ' ')
            o = 1;
        
        if (!strcontains(vars[i], "=")) {
            if (!strcmp(vars[i]+o, "secure")) {
                secure = true;
            }
            continue;
        }
            
        int epos = 0;
        char *k, *v;
        for(epos = 0; vars[i][epos] != '='; epos++);
        k = new char[epos+1];
        memset(k, 0, epos+1);
        strncpy(k, vars[i]+o, epos-o);
        if (strlen(vars[i])-1 == epos) {
            v = new char[2];
            v[0] = '\0';
        }
        else {
            v = new char[strlen(vars[i])-epos+1];
            memset(v, 0, strlen(vars[i])-epos+1);
            strncpy(v, vars[i]+epos+1, strlen(vars[i])-epos+o);
        }
        
        //printf("k, v: [%s, %s]\n", k, v);
        
        /*char** item = strsplit(vars[i]+o, '=');
        if (item[2] != NULL) {
            k = strdup(item[0]);
            v = strjoin("=", item+1);
        }
        else if (item[1] == NULL){
            k = strdup(item[0]);
            v = new char[2];
            v[0] = '\0';
        }
        else {
            k = strdup(item[0]);
            v = strdup(item[1]);
        }
        strdelv(item);*/
        
        /*if (item[0]  == NULL || item[1] == NULL)
            continue;*/
        
        if (!strcmp(k, "domain")) {
            if (domain) {
                free(domain);
                domain = strdup(v);
            }
        }
        else if (!strcmp(k, "path")){
            path = strdup(v);
        }
        else if (!strcmp(k, "expires")) {
            tm t;
            memset(&t, 0, sizeof(t));
            strptime(v, STRPTIME, &t);
            //t.tm_year += 1900;
            expires = mktime(&t);
        }
        else {
            free(key);
            free(value);
            key = strdup(k);
            value = strdup(v);
        }
        
        delete[] k;
        delete[] v;
        //strdelv(item);
    }
    
    strdelv(vars);
    
    //if (path == NULL)
    //    path = strdup("/");
}

Cookie::~Cookie()
{
    free(domain);
    free(path);
    free(key);
    free(value);
}



