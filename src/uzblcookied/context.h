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
        void log(int, std::string);

    protected:
		std::ostream *out;
};

#endif
