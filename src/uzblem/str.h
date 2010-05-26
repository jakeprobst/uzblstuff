#ifndef _STR_H_
#define _STR_H_

#include <string.h>



char** strsplit(char*, char);
char* strjoin(char*, char**);
char* strreplace(char*, char*, char*, int);
char* strdup(char*);

bool strcontains(char*, char*);

int charat(char*, char);

void strdelv(char**);






#endif /* _STR_H_ */
