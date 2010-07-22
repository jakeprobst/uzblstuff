#include "uzblem.h"





int main(int argc, char** argv)
{
    if (argc != 2) {
        printf("usage: %s <socket>\n", argv[0]);
        return 0;
    }
    
    UzblEM* uzblem = new UzblEM(argv[1]);
    uzblem->Run();
    delete uzblem;
    //UzblEM uzblem = UzblEM(argv[1]);
    //uzblem.Run();
}
