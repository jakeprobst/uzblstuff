#include "str.h"


#include <stdio.h>

char** strsplit(char* str, char delim)
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
        if (str[i-1] == delim) {
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

void strdelv(char** s)
{
    for(int i = 0; s[i] != NULL; i++) {
        delete[] s[i];
    }
    delete[] s;
}

char* strjoin(char* d, char** s)
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

char* strreplace(char* str, char* replaceme, char* replaceto, int times)
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

char* strdup(char* s)
{
    char* out = new char[strlen(s)+1];
    memset(out, 0, strlen(s)+1);
    strncpy(out, s, strlen(s));
    return out;
}



bool strcontains(char* h, char* n)
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

int charat(char* str, char c)
{
    int l = strlen(str);
    for(int i = 0; i < l; ++i) {
        if (str[i] == c)
            return i;
    }
    return -1;
}

/*
int main()
{

    char* c = strreplace("uri %s", "%s", "google.com", 1);
    
    printf("[%s]:%d\n", c, strlen(c));
    
    delete[] c;
    
}
*/


