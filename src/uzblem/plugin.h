#ifndef _PLUGIN_H_
#define _PLUGIN_H_

class UzblEM;


class Plugin {
    private:
    
    public:
        //virtual void Load(UzblEM*);
        //virtual void Event(char**) {printf("not overridden!\n");};
        virtual void Event(char**) = 0;
        //virtual voidUnload();
        virtual ~Plugin() {};
    
    
};














#endif /* _PLUGIN_H_ */

