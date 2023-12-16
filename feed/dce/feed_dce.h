
#ifndef  __FEED_DCE_H__
#define  __FEED_DCE_H__

#include "../../modulebase.h"

class Feed_DCE :public ModuleBase
{
public:
    Feed_DCE() { };
    ~Feed_DCE() { };
    
public:
    virtual void start();
    virtual void close();
    static bool running;

private:
    static void run();
};

#endif
