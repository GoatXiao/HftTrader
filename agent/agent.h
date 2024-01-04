#ifndef  __AGENT_STRATEGY_H__
#define  __AGENT_STRATEGY_H__

#include "../modulebase.h"
#include "../api/offerbase.h"

class Agent :public ModuleBase
{
public:
    Agent();
    ~Agent();

    static OfferBase* m_pOspi;
    static Agent* m_pAgent;
    static bool running;
    
public:
    virtual void start();
    virtual void close();

    void handle_cb(const Queue::CBTOA*);
private:
    Queue::qATOL* m_atol_q = nullptr;

    static void run();
    static void run_cb();
public:
    void push_to_log(LOGGER_TYPE type, const Order& order, int userdata = 0, double price = 0, double fee = 0);
private:
    void init();
    void OnSendRtn(uint32_t);
    void OnSendError(uint32_t, int);
    void OnCancelRtn(uint32_t, int);
    void OnCancelError(uint32_t, int);
    void OnTradeRtn(uint32_t, double, int);
};

#endif
