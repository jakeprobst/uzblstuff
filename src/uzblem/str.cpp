#include "str.h"


#include <stdio.h>

/*char** strsplit(const char* str, char delim)
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
            if (str[i] == delim/* || start - lstart == 1* /) {
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
}*/


char** strsplit(const char* str, const char delim)
{
    int len = strlen(str);
    char** out = NULL;

    int itemnum = 2;
    int inquote = -1;
    bool inspace = false;
    for(int i = 0; i < len; i++) {
        if (str[i] == '\'' && str[i-1] != '\\') {
            inquote *= -1;
            if (inquote < 0)
                itemnum++;
        }
        else if (str[i] == delim && inquote < 0) {
            if (inspace)
                continue;
            inspace = true;
            itemnum++;
        }
        else {
            inspace = false;
        }
    }

    out = new char*[itemnum+1];
    for(int i = 0; i < itemnum; i++) {
        out[i] = new char[len];
        memset(out[i], 0, len);
    }
    out[itemnum] = NULL;

    inquote = -1;
    inspace = false;
    int oindex = 0;
    int bindex[itemnum];
    for(int i = 0; i < itemnum; i++)
        bindex[i] = 0;
    for(int i = 0; i < len; i++) {
        if (str[i] == '\\' && str[i+1] == '\\') {
            out[oindex][bindex[oindex]] = str[i];
            bindex[oindex]++;
            inspace = false;
            //i++;
        }
        else if (str[i] == '\\' && str[i+1] == '\'') {
            i++;
        }
        else if (str[i] == '\'') {
            inquote *= -1;
            if (inquote < 0 && bindex[oindex] == 0)
                oindex++;
        }
        else if (str[i] == delim && inquote < 0) {
            if (inspace)
                continue;
            inspace = true;
            oindex++;
        }
        else {
            out[oindex][bindex[oindex]] = str[i];
            bindex[oindex]++;
            inspace = false;
        }
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

char* strreplace(const char* str, const char* replaceme, const char* replaceto, int times)
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

/*char* strdup(const char* s)
{
    char* out = new char[strlen(s)+1];
    memset(out, 0, strlen(s)+1);
    strncpy(out, s, strlen(s));
    return out;
}*/



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

int charat(const char* str, char c)
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


