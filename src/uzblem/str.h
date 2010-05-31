#ifndef _STR_H_
#define _STR_H_

#include <string.h>



char** strsplit(const char*, const char);
char* strjoin(const char*, char**);
char* strreplace(const char*, const char*, const char*, int);
//char* strdup(const char*);

bool strcontains(const char*, const char*);

int charat(const char*, char);

void strdelv(char**);






#endif /* _STR_H_ */
