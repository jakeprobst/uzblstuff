#ifndef _UTIL_H_
#define _UTIL_H_

#include <stdio.h>
#include <string.h>

char* strdup(char*);

char** nullsplit(char*);
char* strjoin(char**);
char* strjoin(char*, char**);

char** strsplit(char*, char);


bool strcontains(char*, char*);
void strdelv(char**);
int strlenv(char**);



bool startswith(char*, char*);
bool endswith(char*, char*);










#endif /* _UTIL_H_ */
