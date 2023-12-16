#ifndef  __STRATEGY_STRATEGY_H__
#define  __STRATEGY_STRATEGY_H__

#include "../modulebase.h"

class Strategy :public ModuleBase
{
public:
    Strategy(int);
    ~Strategy();

public:
    virtual void start();
    virtual void close();

    int m_thread_id;
    bool running;

private:
    static void run(Strategy*);
};

#endif
