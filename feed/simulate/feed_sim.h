#ifndef  __FEED_SIM_H__
#define  __FEED_SIM_H__

#include "../../modulebase.h"

class Feed_SIM :public ModuleBase
{
public:
    Feed_SIM() { };
    ~Feed_SIM() { };
    
public:
    virtual void start();
    virtual void close();
    static bool running;

private:
    static void run();
};

#endif
