#ifndef _CONTEXT_H_
#define _CONTEXT_H_

class Context {
    public:
        Context(int, char **);
        int verbosity;
        bool daemonize;
        bool memory_mode;
        bool help;
};

#endif
