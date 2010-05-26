#include "uzbltreetab.h"


int main(int argc, char** argv)
{
    gtk_init(&argc, &argv);
    srand(time(NULL));
    
    UzblTreeTab* uzbltreetab;
    if (argc > 1)
        uzbltreetab = new UzblTreeTab(argv[1]);
    else
        uzbltreetab = new UzblTreeTab();
    uzbltreetab->Run();
    delete uzbltreetab;
}
