#include <unistd.h>
#include "context.h"

#include <fstream>

Context::Context(int argc, char **argv) {
    verbosity = 0;
    memory_mode = false;
    help = false;
    daemonize = true;

    int index;
    int c;
    while ((c = getopt(argc, argv, "mvfh")) != -1) {
        switch (c) {
            case 'v':
                verbosity++;
                break;
            case 'm':
                memory_mode = true;
                break;
            case 'f':
                daemonize = false;
                break;
            case 'h':
                help = true;
                break;
        }
    }

    if (daemonize)
        out = new std::ofstream("/dev/null", std::ios_base::out);
    else
        out = &std::cerr;
}

void Context::log(int l, std::string s) {
    if (l < verbosity) return;
    *out<<s<<std::endl;
}
