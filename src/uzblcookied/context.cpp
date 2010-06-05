#include <unistd.h>
#include "context.h"

#include <fstream>
#include <cstring>
#include <cerrno>

/** Initializes context based on commandline parameters
  * @param argc Count commandline parameters
  * @param argv Commandline parameters array */
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

/** Log with specified level
  * @param l Log level
  * @param s String to log */
void Context::log(int l, std::string s) {
#ifndef WITHOUT_LOGGING
    if (l > verbosity) return;
    *out<<s<<std::endl;
#endif
}

/** Emulates perror(3) behaviour, but writes to log, whatever it is */
void Context::perror(const char *s) {
	*out<<s<<": "<<strerror(errno)<<std::endl;
}
