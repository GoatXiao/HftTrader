#ifndef  __LOGGER_H__
#define  __LOGGER_H__

#include "../modulebase.h"

class Logger :public ModuleBase
{
public:
    Logger();
    ~Logger();
public:
    virtual void start();
    virtual void close();
private:
    static bool running;
    static void run();
};
#endif
