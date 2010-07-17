#ifndef _COOKIE_H_
#define _COOKIE_H_

#include <map>



class Cookie {
    public:
        //std::map<char*, char*> variables;
        unsigned int expires;
        bool secure;
        char* domain;
        char* path;
        
        char* key;
        char* value;
    
        Cookie(char*); // for loading from cookies.txt
        Cookie(char*, char*); // for loading from socket
        Cookie(const Cookie& other);
        ~Cookie();
        
        
};



bool cookiesort(const Cookie&, const Cookie&);





















#endif /* _COOKIE_H_ */
