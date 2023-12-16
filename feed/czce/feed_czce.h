
#ifndef  __FEED_CZCE_H__
#define  __FEED_CZCE_H__

#include "../../modulebase.h"

class Feed_CZCE :public ModuleBase
{
public:
    Feed_CZCE() { };
    ~Feed_CZCE() { };
    
public:
    virtual void start();
    virtual void close();
    static bool running;

private:
    static void run();
};

#endif
