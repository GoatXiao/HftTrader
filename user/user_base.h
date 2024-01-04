#ifndef  __USER_BASE_H__
#define  __USER_BASE_H__

/*
 *  DO NOT MODIFY
 */

#include "../include.h"

class UserStrategyBase
{
public:
    virtual ~UserStrategyBase(){}
public:
    struct alignas(8) SIGNAL
    {
        UserStrategyBase* p_user;
        InstrumentConfig* p_cfg;
        int64_t ns_data;
        int64_t ns_done;
        double bid;
        double ask;
        int bidvol;
        int askvol;
    };

    // strategy
    virtual int on_new_md(int) = 0;
    virtual uint16_t allocate_size(int) = 0;
    virtual void on_new_event(int, const MdFeed*, uint16_t*, SIGNAL*) = 0;

    // agent
    virtual void on_execute(uint16_t, const SIGNAL*) = 0;

    virtual void on_send_err_rtn(Order*, int) { };
    virtual void on_cancel_rtn(Order*, int) { };
    virtual void on_trade_rtn(Order*, double, int) { };

    virtual int64_t on_delayed_event(int64_t ns) { return ns; };
};
#endif
