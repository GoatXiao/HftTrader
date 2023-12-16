
#ifndef  __FEED_CTP_H__
#define  __FEED_CTP_H__

#include "../../modulebase.h"

class Feed_CTP :public ModuleBase
{
public:
    Feed_CTP() { };
    ~Feed_CTP() { };
    
public:
    virtual void start();
    virtual void close();
    static bool running;

private:
    static void run();
};

#endif
