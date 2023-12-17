#ifndef  __OFFERBASE_H__
#define  __OFFERBASE_H__

#include "../system/system.h"

class OfferBase
{
public:
    OfferBase()
    {
        auto& v_iConfig = SYSTEM::get_inst_cfgs();
        int n = v_iConfig.size();
        m_vState.reserve(n);
        m_mState.clear();
        for (int i = 0; i < n; ++i) 
        {
            auto& cfg = v_iConfig[i];
            InstrumentState state(&cfg);
            memcpy((void*)(&(m_vState[i])), (void*)(&state), sizeof(InstrumentState));
            const char* inst = cfg.inst.data();
            m_mState.emplace(inst, &m_vState[i]);
        }
        m_mState.doneModify();
        m_mState.clear();

        for (uint32_t i = 0; i < ORDERPOOL_NUM; ++i)
        {
            OrderPool[i].orderid = i;
        }
    }

    virtual ~OfferBase(){}

	virtual bool start() { return true; };
	virtual bool stop() { return true; };

    virtual bool send_order(Order& order) { return false; };
    virtual bool cancel_order(uint32_t orderid) { return false; };
    virtual int64_t get_sysorderid(uint32_t orderid) { return -1; };

public:
    inline InstrumentState* get(int idx)
    {
        return &m_vState[idx];
    };

    inline InstrumentState* get(const char* inst)
    {
        return m_mState.fastFind(inst);
    };

    inline Order& get_order(int idx)
    {
        return OrderPool[idx];
    };

    inline Order& get_next_order()
    {
        return OrderPool[++OrderID];
    };

    inline void set_orderid(int id)
    {
        OrderID = id;
    };

public:
    bool cancel(Order& o, int n) 
    {
        if (o.ns_recv == 0) {
            o.ns_recv = TIMER::tsc();
        }
        int remain = o.volume - o.filled;
        n = std::min(n, remain);
        if (n <= 0) { return false; }

        auto* state = get(o.inst_id);
        switch (o.direction) {
        case 'b':
            state->erase_price<'b'>(o.price, n);
            state->erase_orderid<'b'>(o.orderid);
            break;
        case 's':
            state->erase_price<'s'>(o.price, n);
            state->erase_orderid<'s'>(o.orderid);
            break;
        }
        o.status = ORDER_STATUS::O_CANCELED;

        auto* p_cfg = state->p_cfg;
        if (o.fak == 0) {
            p_cfg->num_cancel_gfd++;
        } else {
            p_cfg->num_cancel_fak++;
        }
        p_cfg->num_info++;
        return true;
    };

    void cancel_error(Order& o) 
    {
        auto* state = get(o.inst_id);
        auto* p_cfg = state->p_cfg;
        p_cfg->num_cancel_err++;
        p_cfg->num_info++;
    };

    void send_error(Order& o) 
    {
        if (o.ns_recv == 0) {
            o.ns_recv = TIMER::tsc();
        }
        auto* state = get(o.inst_id);
        int remain = o.volume - o.filled;
        switch (o.direction) {
        case 'b':
            state->erase_price<'b'>(o.price, remain);
            state->erase_orderid<'b'>(o.orderid);
            break;
        case 's':
            state->erase_price<'s'>(o.price, remain);
            state->erase_orderid<'s'>(o.orderid);
            break;
        }
        o.status = ORDER_STATUS::O_ERROR;

        auto* p_cfg = state->p_cfg;
        p_cfg->num_insert_err++;
        p_cfg->num_insert--;
    };

    void confirm(Order& o) 
    {
        ORDER_STATUS status = (ORDER_STATUS)o.status;
        if (ORDER_STATUS::O_CANCELED == status or 
            ORDER_STATUS::O_COMPLETE == status) 
        {
            return;
        }

        if (o.ns_recv == 0) {
            o.ns_recv = TIMER::tsc();
        }

        auto* state = get(o.inst_id);
        switch (o.direction) {
        case 'b':
            state->insert_orderid<'b'>(o.orderid, o.price);
            break;
        case 's':
            state->insert_orderid<'s'>(o.orderid, o.price);
            break;
        }
        o.status = ORDER_STATUS::O_QUEUEING;
    };

    bool trade(Order& o, double pr, int n, double& fee) 
    {
        if (o.ns_recv == 0) {
            o.ns_recv = TIMER::tsc();
        }
        o.filled += n;
        int remain = o.volume - o.filled;
        auto* state = get(o.inst_id);
        switch (o.direction) {
        case 'b':
            state->update<'b'>(o.offset, pr, n, fee);
            state->erase_price<'b'>(o.price, n);
            if (remain <= 0) {
                state->erase_orderid<'b'>(o.orderid);
            }
            break;
        case 's':
            state->update<'s'>(o.offset, pr, n, fee);
            state->erase_price<'s'>(o.price, n);
            if (remain <= 0) {
                state->erase_orderid<'s'>(o.orderid);
            }
            break;
        }
        if (remain <= 0) {
            o.status = ORDER_STATUS::O_COMPLETE;
            return true;
        }
        return false;
    };

    void send(Order& o) {
        auto* state = get(o.inst_id);
        o.ns_init = state->ns_signal;
        o.time = state->timestamp;
        switch (o.direction) {
        case 'b':
            state->insert_price<'b'>(o.price, o.volume);
            break;
        case 's':
            state->insert_price<'s'>(o.price, o.volume);
            break;
        }
        o.status = ORDER_STATUS::O_SEND;
        
        auto* p_cfg = state->p_cfg;
        p_cfg->num_insert++;
        p_cfg->num_info++;
    };

    template<char d, bool g> bool set(Order& o) 
    {
        auto* state = get(o.inst_id);
        if (g) {
            int n_fak = 0;
            switch (d) {
            case 'b':
                n_fak = state->get_outstanding_volume<'s'>(o.price);
                if (n_fak > 0) {
                    o.volume += n_fak;
                    o.fak = 1;
                }
                break;
            case 's':
                n_fak = state->get_outstanding_volume<'b'>(o.price);
                if (n_fak > 0) {
                    o.volume += n_fak;
                    o.fak = 1;
                }
                break;
            }
        }
        o.offset = state->get_offset<d>(
            o.volume + state->get_outstanding_volume<d>()
        );
        return ORDER_OFFSET::O_INVALID != (ORDER_OFFSET)o.offset;
    };

protected:
    std::vector<InstrumentState> m_vState;
    HashMap<InstrumentState>::type m_mState;
    Order OrderPool[ORDERPOOL_NUM];
    uint32_t OrderID = 0;
};
#endif
