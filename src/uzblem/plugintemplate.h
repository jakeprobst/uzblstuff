#ifndef _PLUGINTEMPLATE_H_
#define _PLUGINTEMPLATE_H_

#include "plugin.h"

class UzblEM;

class PluginTemplate: public Plugin {
    private:
        UzblEM* em;
    
    public:
        PluginTemplate(UzblEM*);
        ~PluginTemplate();
        
        
        void Event(char**);
};













#endif /* _PLUGINTEMPLATE_H_ */
