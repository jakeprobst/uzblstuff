#ifndef _CONF_H_
#define _CONF_H_

class Conf {
    public:
        Conf(int, char **);
        int verbosity;
        bool memory_mode;
        bool help;
};

#endif
