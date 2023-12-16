
#ifndef  __FEED_SHFE_H__
#define  __FEED_SHFE_H__

#include "../../modulebase.h"

class Feed_SHFE :public ModuleBase
{
public:
    Feed_SHFE() { };
    ~Feed_SHFE() { };
    
public:
    virtual void start();
    virtual void close();
    static bool running;

private:
    static void run();
};

#endif
