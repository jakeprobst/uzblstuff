#include "util.h"

char* strjoin(char** s)
{
    int len = 0;
    int items = 0;
    for(int i = 0; s[i] != NULL; i++) {
        //printf("%s: %d\n", s[i], strlen(s[i]));
        len += strlen(s[i]);
        items++;
    }
    
    //len += strlen(d)*(items-1) + 1;
    len++;
    
    //printf("len: %d\n", len);
    char* out = new char[len];
    memset(out, 0, len);
    
    
    for(int i = 0; i < items; i++) {
        strncat(out,s[i],len);
    }
    
    
    return out;
}

char* strjoin(const char* d, char** s)
{
    int len = 0;
    int items = 0;
    for(int i = 0; s[i] != NULL; i++) {
        len += strlen(s[i]);
        items++;
    }
    
    len += strlen(d)*(items-1) + 1;
    
    char* out = new char[len];
    memset(out, 0, len);
    
    
    for(int i = 0; i < items-1; i++) {
        strncat(out,s[i],len);
        strncat(out, d, len);
    }
    strncat(out,s[items-1],len);
    
    
    return out;
}

char** nullsplit(const char* str)
{
    int items = 1;
    for(int i = 0;;i++) {
        if (str[i] == '\0') {
            if (str[i+1] == '\0')
                break;
            items++;
        }
    }
    
    int itemlen[items];
    int c = 0, n = 0;
    for(int i = 0;;i++) {
        if (str[i] == '\0') {
            if (str[i+1] == '\0') {
                itemlen[n++] = c+2;
                break;
            }
            itemlen[n++] = c+2;
            c = 0;
        }
        c++;
    }
    
    char** out = new char*[items+1];
    for(int i = 0; i < items; i++) {
        out[i] = new char[itemlen[i]];
        memset(out[i], 0, itemlen[i]);
    }
    out[items] = NULL;
    
    c = n = 0;
    for(int i = 0;;i++) {
        if (str[i] == '\0') {
            if (str[i+1] == '\0')
                break;
            n++;
            c = 0;
            continue;
        }
        out[n][c++] = str[i];
    }
    
    return out;
}

char** strsplit(const char* str, char delim)
{
    int len = strlen(str);
    char** out;
    
    int items = 1;
    bool dhit = false;
    for(int i = 0; i < len; i++) {
        if (str[i] == delim )
            dhit = true;
        else if (dhit) {
            items++;
        }
    }
    out = new char*[items+1]; // NULL end
    for(int i = 0; i < items+1; i++)
        out[i] = NULL;
    
    
    int start = 0;
    int lstart = 0;
    int o = 0;
    int spaceoff = 0;
    for(int i = 0; true; i++) {
        if (i == len) {
            if (spaceoff)
                spaceoff++;
            out[o] = new char[len - lstart +1 -spaceoff];
            memset(out[o], 0, len - lstart +1 -spaceoff);
            strncpy(out[o], str+lstart, len - lstart -spaceoff);
            
            break;
        }
        if (i != 0 && str[i-1] == delim) {
            if (str[i] == delim/* || start - lstart == 1*/) {
                start++;
                spaceoff++;
                continue;
            }
            out[o] = new char[start - lstart + 1 - spaceoff];
            memset(out[o], 0, start - lstart + 1 - spaceoff);
            strncpy(out[o], str+lstart, start - lstart-1 - spaceoff);
            
            spaceoff = 0;
            lstart = start;
            o++;
        }
        
        
        start++;
    }
    
    return out;
}


bool strcontains(const char* h, const char* n)
{
    int hlen = strlen(h);
    int nlen = strlen(n);
    
    for(int i = 0; i < hlen; i++) {
        if (!strncmp(h+i, n, nlen)) {
            return true;
        }
    }
    
    return false;
}

void strdelv(char** s)
{
    for(int i = 0; s[i] != NULL; i++) {
        delete[] s[i];
    }
    delete[] s;
}

int strlenv(char** s)
{
    int i;
    for(i = 0; s[i] != NULL; i++);
    return i;
}


bool startswith(const char* h, const char* n)
{
    if (!strncmp(h, n, strlen(n)))
        return true;
    return false;
}

bool endswith(const char* h, const char* n)
{
    int ho = strlen(h) - strlen(n);
    if (!strncmp(h+ho, n, strlen(n)))
        return true;
    return false;
}






