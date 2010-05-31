#include <unistd.h>
#include "conf.h"

Conf::Conf(int argc, char **argv) {
    verbosity = 0;
    memory_mode = false;
    help = false;

    int index;
    int c;
    while ((c = getopt(argc, argv, "mvh")) != -1) {
        switch (c) {
            case 'v':
                verbosity++;
                break;
            case 'm':
                memory_mode = true;
                break;
            case 'h':
                help = true;
                break;
        }
    }
}
