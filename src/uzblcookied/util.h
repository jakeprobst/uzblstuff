#ifndef _UTIL_H_
#define _UTIL_H_

#include <stdio.h>
#include <string.h>

char* strdup(const char*);

char** nullsplit(const char*);
char* strjoin(char**);
char* strjoin(const char*, char**);

char** strsplit(const char*, char);


bool strcontains(const char*, const char*);
void strdelv(char**);
int strlenv(char**);



bool startswith(const char*, const char*);
bool endswith(const char*, const char*);










#endif /* _UTIL_H_ */
