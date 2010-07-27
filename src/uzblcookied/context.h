#ifndef _CONTEXT_H_
#define _CONTEXT_H_

#include <iostream>
#include <string>

class Context {
    public:
        Context(int, char **);
        int verbosity;
        bool daemonize;
        bool memory_mode;
        bool help;
        bool writerequest;
        bool nowrite;
        bool running;
        void log(int, std::string);
        void perror(const char *);

    protected:
        std::ostream *out;
};

#endif
