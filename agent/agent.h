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

private:
    void init();

    template<LOGGER_TYPE>
    void push_to_log(const Order&, int userdata = 0, double price = 0, double fee = 0);

    void OnSendRtn(uint32_t);
    void OnSendError(uint32_t, int);
    void OnCancelRtn(uint32_t, int);
    void OnCancelError(uint32_t, int);
    void OnTradeRtn(uint32_t, double, int);
};

#endif
