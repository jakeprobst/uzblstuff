#include "uzbltab.h"








int main(int argc, char** argv)
{
    gtk_init(&argc, &argv);
    
    
    UzblTab* uzbltab = new UzblTab();
    uzbltab->Run();
    delete uzbltab;
    
    
    
    
}
