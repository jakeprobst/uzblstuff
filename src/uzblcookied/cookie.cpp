#include <time.h>
#include <stdlib.h>

#include "cookie.h"
#include "util.h"

const char* STRPTIME = "%a, %d-%b-%Y %H:%M:%S %Z";
//Fri, 22-May-2020 03:25:25 GMT

bool cookiesort(const Cookie& a, const Cookie& b)
{
    int res;
#define COMPARE(type) \
    res = strcmp(a.type, b.type); \
    if (res != 0) \
        return (res < 0) ? true : false;
    COMPARE(domain);
    COMPARE(path);
    COMPARE(key);
#undef COMPARE
    // These properties aren't compared: value, secure, expires
    return false;
}
//EVENT [142606374] ADD_COOKIE '.google.com' '/' 'PREF' 'ID=47cab6979f111161:FF=0:TM=1297855198:LM=1297855198:S=YJBY6mREZvB_79qm' 'http' '1360927198'

Cookie::Cookie(const Cookie& other)
{
    data = strdup(other.data);
    expires = other.expires;
    secure = other.secure;
    domain = strdup(other.domain);
    path = strdup(other.path);
    key = strdup(other.key);
    value = strdup(other.value);
}

Cookie::Cookie(const char* d)
{
    char name[512];
    char tdomain[512];
    char tbleh[32];
    char tpath[512];
    char tsecure[32];
    int texpires;
    char tkey[1024*4];
    char tvalue[1024*4];

    sscanf(d, "'%[^']' '%[^']' '%[^']' '%[^']' '%[^']' '%d'",
            tdomain, tpath, tkey, tvalue, tsecure, &texpires);
    /*printf("cookie:\n");
    printf("  data: [%s]\n", d);
    printf("  domain: %s\n", tdomain);
    printf("  path: %s\n", tpath);
    printf("  key: %s\n", tkey);
    printf("  value: %s\n", tvalue);
    printf("  sec: %s\n", tsecure);
    printf("  exp: %d\n", texpires);*/
    
    data = strdup(d);
    domain = strdup(tdomain);
    path = strdup(tpath);
    secure = false;
    if (!strcmp(tsecure, "https"))
        secure = true;
    expires = texpires;
    key = strdup(tkey);
    value = strdup(tvalue);
}

Cookie::Cookie(const char* host, const char* data)
{
    /*domain = strdup(host);
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
    }
    
    strdelv(vars);*/
}

Cookie::~Cookie()
{
    free(data);
    free(domain);
    free(path);
    free(key);
    free(value);
}



