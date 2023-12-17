#ifndef  __USER_DERIVED_H__
#define  __USER_DERIVED_H__

#include "user_base.h"


/*
 *  Anything that you wanna pass from on_new_event() to on_execute().
 *
 *  NOTE: 
 *  on_new_event() and on_execute() reside in the same class, 
 *  BUT they run in separate CPUs. 
 *
 *  Be aware of data locality.
 */
struct alignas(8) USER_MSG :public UserStrategyBase::SIGNAL
{
    int inst_id;
    int signal;
};

/*
 *  Anything that you wanna store locally for fast retrieval
 */
struct alignas(8) USER_BUFFER :public MdFeed
{
};


class UserStrategy :public UserStrategyBase
{
public:
    UserStrategy(const ThreadConfig*);
    virtual ~UserStrategy();
    
public:
    /* Must */
    int on_new_md(int);
    uint16_t allocate_size(int);
    void on_new_event(int, const MdFeed*, uint16_t*, SIGNAL*);
    void on_execute(uint16_t, const SIGNAL*);

    /* Optional */
    void on_send_err_rtn(Order*, int);
    void on_cancel_rtn(Order*, int);
    void on_trade_rtn(Order*, double, int);

    int64_t on_delayed_event(int64_t);

private:
    const ThreadConfig* const m_pcfg;

    std::vector<int> m_vIdx;
    std::vector<USER_BUFFER> m_vBuffer;
    std::queue<USER_BUFFER*> delayed_queue;

private:
    /* Overload LOG() methods as desired */
    void LOG(int, int);
};
#endif
