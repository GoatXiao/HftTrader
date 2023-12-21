
#include "../api/ctp2mini/ctp2miniOSpi.h"
#include "../api/simulate/simulateOSpi.h"
#include "../api/xele/xeleOSpi.h"
#include "../api/v10/v10OSpi.h"
#include "../api/yd/ydOSpi.h"

#include "../user.h"
#include "agent.h"

#ifdef __SIMULATE
#include "../tool/tool.h"
#endif

bool Agent::running = true;

Agent* Agent::m_pAgent = nullptr;
OfferBase* Agent::m_pOspi = nullptr;

namespace TRADER 
{
    void set_OrderID(int id) 
    {
        Agent::m_pOspi->set_orderid(id);
    };

    void on_off(const std::vector<std::string>& cmd, bool flag)
    {
        if (cmd.size() == 1 and cmd[0] == "all") 
        {
            const auto& gConfig = SYSTEM::get_system_cfg();
            for (const auto& v : gConfig.inst_list) {
                volatile auto* p = Agent::m_pOspi->get(v.c_str());
                if (p) { p->on_off = flag; }
            }
        } 
        else 
        {
            for (const auto& v : cmd) {
                volatile auto* p = Agent::m_pOspi->get(v.c_str());
                if (p) { p->on_off = flag; }
            }
        }
    };

    void print(const std::vector<std::string>& cmd)
    {
        if (cmd.size() == 1 and cmd[0] == "all") 
        {
            const auto& gConfig = SYSTEM::get_system_cfg();
            for (const auto& v : gConfig.inst_list) {
                const auto* p = Agent::m_pOspi->get(v.c_str());
                if (p) { p->print(); }
            }
        } 
        else 
        {
            for (const auto& v : cmd) {
                const auto* p = Agent::m_pOspi->get(v.c_str());
                if (p) { p->print(); }
            }
        }
    };

    Order& get_order(uint32_t orderid) 
    {
        return Agent::m_pOspi->get_order(orderid);
    };

    int64_t get_sysorderid(uint32_t orderid) 
    {
        return Agent::m_pOspi->get_sysorderid(orderid);
    };

    int get_buy_outstanding_volume(int inst_id) 
    {
        auto* state = Agent::m_pOspi->get(inst_id);
        return state->get_outstanding_volume<'b'>();
    };

    int get_sell_outstanding_volume(int inst_id)
    {
        auto* state = Agent::m_pOspi->get(inst_id);
        return state->get_outstanding_volume<'s'>();
    };

    int get_buy_outstanding_volume(int inst_id, double price) 
    {
        auto* state = Agent::m_pOspi->get(inst_id);
        return state->get_outstanding_volume<'b'>(price);
    };

    int get_sell_outstanding_volume(int inst_id, double price)
    {
        auto* state = Agent::m_pOspi->get(inst_id);
        return state->get_outstanding_volume<'s'>(price);
    };

    template<char direct>
    void handle_outstanding_order(int inst_id, std::function<void(const Order&)> func)
    {
        auto* state = Agent::m_pOspi->get(inst_id);
        const auto& map = state->get_outstanding_order<direct>();
        for (const auto& iter : map) 
        { 
            func(get_order(iter.first)); 
        }
    };

    uint32_t send_order(int inst_id, double price, int volume, uint8_t fak, char direction, char offset, void* __this)
    {
        auto& order = Agent::m_pOspi->get_next_order();

        order.inst_id = inst_id;
        order.pUser = __this;
        order.price = price;
        order.volume = volume;
        order.fak = fak;
        order.direction = direction;
        order.offset = offset;

        if (Agent::m_pOspi->send_order(order))
        {
            Agent::m_pOspi->send(order);
            Agent::m_pAgent->push_to_log(LOGGER_TYPE::SEND_LOG, order);
            return order.orderid;
        }
        return 0;
    };

    bool cancel_order(uint32_t orderid)
    {
        if (Agent::m_pOspi->cancel_order(orderid))
        {
            auto& order = Agent::m_pOspi->get_order(orderid);
            order.status = ORDER_STATUS::O_CANCELING;
            Agent::m_pAgent->push_to_log(LOGGER_TYPE::CANCEL_LOG, order);
            return true;
        }
        return false;
    };

    void cancel_buy_order(int inst_id, double price) 
    {
        auto* state = Agent::m_pOspi->get(inst_id);
        if (0 == state->get_outstanding_volume<'b'>()) { return; }
        auto& orders = state->get_outstanding_order<'b'>();
        int pr = std::nearbyint(price * state->inv_ptick);
        for (const auto& iter : orders) {
            if (iter.second == pr or pr == 0)
            {
                auto& order = Agent::m_pOspi->get_order(iter.first);
                if (ORDER_STATUS::O_QUEUEING == (ORDER_STATUS)order.status)
                {
                    if (Agent::m_pOspi->cancel_order(order.orderid))
                    {
                        order.status = ORDER_STATUS::O_CANCELING;
                        Agent::m_pAgent->push_to_log(LOGGER_TYPE::CANCEL_LOG, order);
                    }
                }
            }
        }
    };

    void cancel_sell_order(int inst_id, double price)
    {
        auto* state = Agent::m_pOspi->get(inst_id);
        if (0 == state->get_outstanding_volume<'s'>()) { return; }
        auto& orders = state->get_outstanding_order<'s'>();
        int pr = std::nearbyint(price * state->inv_ptick);
        for (const auto& iter : orders) {
            if (iter.second == pr or pr == 0)
            {
                auto& order = Agent::m_pOspi->get_order(iter.first);
                if (ORDER_STATUS::O_QUEUEING == (ORDER_STATUS)order.status)
                {
                    if (Agent::m_pOspi->cancel_order(order.orderid))
                    {
                        order.status = ORDER_STATUS::O_CANCELING;
                        Agent::m_pAgent->push_to_log(LOGGER_TYPE::CANCEL_LOG, order);
                    }
                }
            }
        }
    };

    uint32_t send_buy_order_selftradeguard(int inst_id, double price, int volume, void* __this)
    {
        auto& order = Agent::m_pOspi->get_next_order();
        
        order.inst_id = inst_id;
        order.pUser = __this;
        order.price = price;
        order.volume = volume;
        order.direction = 'b';
        
        if (Agent::m_pOspi->set<'b', true>(order))
        {
            if (Agent::m_pOspi->send_order(order))
            {
                Agent::m_pOspi->send(order);
                Agent::m_pAgent->push_to_log(LOGGER_TYPE::SEND_LOG, order);
                return order.orderid;
            }
        }
        return 0;
    };

    uint32_t send_buy_order_noselftradeguard(int inst_id, double price, int volume, void* __this)
    {
        auto& order = Agent::m_pOspi->get_next_order();

        order.inst_id = inst_id;
        order.pUser = __this;
        order.price = price;
        order.volume = volume;
        order.direction = 'b';

        if (Agent::m_pOspi->set<'b', false>(order))
        {
            if (Agent::m_pOspi->send_order(order))
            {
                Agent::m_pOspi->send(order);
                Agent::m_pAgent->push_to_log(LOGGER_TYPE::SEND_LOG, order);
                return order.orderid;
            }
        }
        return 0;
    };

    uint32_t send_sell_order_selftradeguard(int inst_id, double price, int volume, void* __this)
    {
        auto& order = Agent::m_pOspi->get_next_order();

        order.inst_id = inst_id;
        order.pUser = __this;
        order.price = price;
        order.volume = volume;
        order.direction = 's';

        if (Agent::m_pOspi->set<'s', true>(order))
        {
            if (Agent::m_pOspi->send_order(order))
            {
                Agent::m_pOspi->send(order);
                Agent::m_pAgent->push_to_log(LOGGER_TYPE::SEND_LOG, order);
                return order.orderid;
            }
        }
        return 0;
    };

    uint32_t send_sell_order_noselftradeguard(int inst_id, double price, int volume, void* __this)
    {
        auto& order = Agent::m_pOspi->get_next_order();

        order.inst_id = inst_id;
        order.pUser = __this;
        order.price = price;
        order.volume = volume;
        order.direction = 's';

        if (Agent::m_pOspi->set<'s', false>(order))
        {
            if (Agent::m_pOspi->send_order(order))
            {
                Agent::m_pOspi->send(order);
                Agent::m_pAgent->push_to_log(LOGGER_TYPE::SEND_LOG, order);
                return order.orderid;
            }
        }
        return 0;
    };

    uint32_t close_net_position(int inst_id, void* __this) 
    {
        const auto* state = Agent::m_pOspi->get(inst_id);
        const auto* p_cfg = state->p_cfg;
        if (p_cfg->position > 0)//多头净头寸
        {
            cancel_buy_order(inst_id);
            cancel_sell_order(inst_id);

            auto& order = Agent::m_pOspi->get_next_order();

            order.inst_id = inst_id;
            order.pUser = __this;
            order.price = state->bid;
            order.volume = p_cfg->position;
            order.direction = 's';
#ifdef __SIMULATE
            order.force = 1;
#endif
            if (Agent::m_pOspi->set<'s', true>(order))
            {
                if (Agent::m_pOspi->send_order(order))
                {
                    Agent::m_pOspi->send(order);
                    Agent::m_pAgent->push_to_log(LOGGER_TYPE::SEND_LOG, order);
                    return order.orderid;
                }
            }
        }
        else if (p_cfg->position < 0)//空头净头寸
        {
            cancel_sell_order(inst_id);
            cancel_buy_order(inst_id);

            auto& order = Agent::m_pOspi->get_next_order();

            order.inst_id = inst_id;
            order.pUser = __this;
            order.price = state->ask;
            order.volume = -p_cfg->position;
            order.direction = 'b';
#ifdef __SIMULATE
            order.force = 1;
#endif
            if (Agent::m_pOspi->set<'b', true>(order))
            {
                if (Agent::m_pOspi->send_order(order))
                {
                    Agent::m_pOspi->send(order);
                    Agent::m_pAgent->push_to_log(LOGGER_TYPE::SEND_LOG, order);
                    return order.orderid;
                }
            }
        }
        return 0;
    };

    template void handle_outstanding_order<'b'>(int inst_id, std::function<void(const Order&)> func);
    template void handle_outstanding_order<'s'>(int inst_id, std::function<void(const Order&)> func);
} // namespace


Agent::Agent()
{
}

Agent::~Agent()
{
    delete m_pOspi;
    m_pOspi = nullptr;
}

void Agent::close()
{
    *(volatile bool*)&Agent::running = false;
}

void Agent::start()
{
    init();

    std::thread t_Agent(Agent::run);
    t_Agent.detach();

    std::thread t_CB(Agent::run_cb);
    t_CB.detach();
}

void Agent::init()
{
    if (!m_pOspi)
    {
#ifdef __OFFER_CTP2MINI
        m_pOspi = new Ctp2MiniOSpi(false, false, QRY_INSTRUMENT_TYPE::FUTURE_ONLY);
#endif

#ifdef __SIMULATE
        m_pOspi = new SimulateOSpi();
#endif

#ifdef __OFFER_YD
        m_pOspi = new YDOSpi();
#endif

#ifdef __OFFER_XELE
        m_pOspi = new XeleOSpi();
#endif

#ifdef __OFFER_V10
        m_pOspi = new V10OSpi(m_cbtoa_q, &m_mState);
#endif
    }

    m_atol_q = QUEUE::get_agent2log();
    m_pAgent = this;
}

void Agent::run_cb()
{
    SYSTEM::bind_cpuid(CPUID::CALLBACK_CPUID, 0);
    if (m_pOspi)
    {
        m_pOspi->start();
    }
}

void Agent::handle_cb(const Queue::CBTOA* cbtoa)
{
    switch ((CALLBACK_TYPE)cbtoa->msg_type) 
    {
    case CALLBACK_TYPE::ORDER_CONFIRM:
    {
        OnSendRtn(cbtoa->reference_id);
        break;
    }
    case CALLBACK_TYPE::ORDER_ERROR:
    {
        OnSendError(cbtoa->reference_id, cbtoa->errid);
        break;
    }
    case CALLBACK_TYPE::ORDER_CANCEL_ERR:
    {
        OnCancelError(cbtoa->reference_id, cbtoa->errid);
        break;
    }
    case CALLBACK_TYPE::ORDER_CANCEL:
    {
        OnCancelRtn(cbtoa->reference_id, cbtoa->volume);
        break;
    }
    case CALLBACK_TYPE::ORDER_TRADE:
    {
        OnTradeRtn(cbtoa->reference_id, cbtoa->price, cbtoa->volume);
        break;
    }
    default:
        break;
    }
}

void Agent::OnSendRtn(uint32_t orderid)
{
    auto& order = Agent::m_pOspi->get_order(orderid);
    Agent::m_pOspi->confirm(order);
}

void Agent::OnCancelRtn(uint32_t orderid, int num)
{
    auto& order = Agent::m_pOspi->get_order(orderid);
    if (Agent::m_pOspi->cancel(order, num))
    {
        // user func
        auto* pUser = (UserStrategyBase*)order.pUser;
        pUser->on_cancel_rtn(&order, num);
        push_to_log(LOGGER_TYPE::COMPLETE_LOG, order);
    }

}

void Agent::OnTradeRtn(uint32_t orderid, double pr, int v)
{
    auto& order = Agent::m_pOspi->get_order(orderid);

    double fee = 0;
    bool done = Agent::m_pOspi->trade(order, pr, v, fee);

    // user func
    auto* pUser = (UserStrategyBase*)order.pUser;
    pUser->on_trade_rtn(&order, pr, v);
    push_to_log(LOGGER_TYPE::TRADE_LOG, order, v, pr, fee);
    
    if (done) {
        push_to_log(LOGGER_TYPE::COMPLETE_LOG, order);
    }
}

void Agent::OnCancelError(uint32_t orderid, int errid)
{
    auto& order = Agent::m_pOspi->get_order(orderid);
    Agent::m_pOspi->cancel_error(order);

    push_to_log(LOGGER_TYPE::ERR_LOG, order, errid);
}

void Agent::OnSendError(uint32_t orderid, int errid)
{
    auto& order = Agent::m_pOspi->get_order(orderid);
    Agent::m_pOspi->send_error(order);

    // user func
    auto* pUser = (UserStrategyBase*)order.pUser;
    pUser->on_send_err_rtn(&order, errid);

    push_to_log(LOGGER_TYPE::ERR_LOG, order, errid);
}

void Agent::push_to_log(LOGGER_TYPE type, const Order& order, int userdata, double price, double fee)
{
    const auto* state = Agent::m_pOspi->get(order.inst_id);
    const auto* p_cfg = state->p_cfg;
    int64_t ns = TIMER::tsc();
    switch ((uint16_t)type)
    {
    case LOGGER_TYPE::COMPLETE_LOG:
    {
        Queue::qATOL::MsgHeader* header = nullptr;
        do 
        { 
            header = m_atol_q->alloc(sizeof(Queue::ORDER_LOG_TYPE));
        } while (!header);
        header->msg_type = (uint16_t)type;
        header->userdata = state->timestamp;
        auto* data = (Queue::ORDER_LOG_TYPE*)(header + 1);
        data->order = &order;
        data->num_insert = p_cfg->num_insert;
        data->num_insert_err = p_cfg->num_insert_err;
        data->num_cancel_gfd = p_cfg->num_cancel_gfd;
        data->num_cancel_fak = p_cfg->num_cancel_fak;
        data->num_cancel_err = p_cfg->num_cancel_err;
        data->num_info = p_cfg->num_info;
        data->ns = ns;
        m_atol_q->push();
        break;
    }
    case LOGGER_TYPE::TRADE_LOG:
    {
        Queue::qATOL::MsgHeader* header = nullptr;
        do 
        { 
            header = m_atol_q->alloc(sizeof(Queue::TRADE_LOG_TYPE));
        } while (!header);
        header->msg_type = (uint16_t)type;
        header->userdata = state->timestamp;
        auto* data = (Queue::TRADE_LOG_TYPE*)(header + 1);
        data->inst_id = order.inst_id;
        data->localid = order.orderid;
        data->insert_time = order.time;
        data->price = price * p_cfg->Multiple;
        data->fee = fee * p_cfg->Multiple;
        data->direction = order.direction;
        data->offset = order.offset;
        data->volume = userdata;
        data->ns = ns;
        m_atol_q->push();
        break;
    }
    case LOGGER_TYPE::SEND_LOG:
    {
        Queue::qATOL::MsgHeader* header = nullptr;
        do 
        { 
            header = m_atol_q->alloc(sizeof(Queue::SEND_LOG_TYPE));
        } while (!header);
        header->msg_type = (uint16_t)type;
        header->userdata = state->timestamp;
        auto* data = (Queue::SEND_LOG_TYPE*)(header + 1);
        data->inst_id = order.inst_id;
        data->localid = order.orderid;
        data->price = order.price;
        data->direction = order.direction;
        data->offset = order.offset;
        data->volume = order.volume;
        data->fak = order.fak;
        data->ns = ns;
        m_atol_q->push();
        break;
    }
    case LOGGER_TYPE::CANCEL_LOG:
    {
        Queue::qATOL::MsgHeader* header = nullptr;
        do 
        { 
            header = m_atol_q->alloc(sizeof(Queue::CANCEL_LOG_TYPE));
        } while (!header);
        header->msg_type = (uint16_t)type;
        header->userdata = state->timestamp;
        auto* data = (Queue::CANCEL_LOG_TYPE*)(header + 1);
        data->inst_id = order.inst_id;
        data->localid = order.orderid;
        data->insert_time = order.time;
        data->ns = ns;
        m_atol_q->push();
        break;
    }
    case LOGGER_TYPE::ERR_LOG:
    {
        Queue::qATOL::MsgHeader* header = nullptr;
        do 
        { 
            header = m_atol_q->alloc(sizeof(Queue::ERR_LOG_TYPE));
        } while (!header);
        header->msg_type = (uint16_t)type;
        header->userdata = state->timestamp;
        auto* data = (Queue::ERR_LOG_TYPE*)(header + 1);
        data->inst_id = order.inst_id;
        data->localid = order.orderid;
        data->time = order.time;
        data->errid = userdata;
        data->ns = ns;
        m_atol_q->push();
        break;
    }
    }
}

void Agent::run()
{
    SYSTEM::bind_cpuid(CPUID::AGENT_CPUID, THREAD_PRIORITY::THREAD_AGENT);

    auto* qcbtoa = QUEUE::get_api2agent();
    auto& v_qutoa = (const std::vector<Queue::qUTOA*>)QUEUE::get_user2agent();

    while (Agent::running)
    {
        for (auto* q : v_qutoa) 
        {
            q->tryPop(
                [&](Queue::qUTOA::MsgHeader* header) 
                {
                    int64_t ns = TIMER::tsc();
                    auto* p = (UserStrategyBase::SIGNAL*)(header + 1);
                    const auto* p_cfg = p->p_cfg;
                    auto* p_user = p->p_user;

                    int inst_id = p_cfg->inst_id;
                    auto* state = Agent::m_pOspi->get(inst_id);
                    state->ns_data = p->ns_data; //行情抵达时间
                    state->ns_signal = p->ns_done; //策略完成

                    state->set(
                        header->userdata, 
                        p->bid, p->ask, 
                        p->bidvol, p->askvol
                    );

#ifdef __SIMULATE
                    Agent::m_pOspi->handle_order(inst_id);
                    while (
                        qcbtoa->tryPop(
                            [&](Queue::CBTOA* cbtoa) {
                                Agent::m_pAgent->handle_cb(cbtoa);
                            }
                        )
                    ); // exhaustive
#endif
                    p_user->on_execute(header->msg_type, p);
                    state->guard(); //事后风控
#ifdef __SIMULATE
                    Tools::unlock_sim();
#endif
                }
            );
        }

#ifndef __SIMULATE
        qcbtoa->tryPop(
            [&](Queue::CBTOA* cbtoa) 
            {
                Agent::m_pAgent->handle_cb(cbtoa);
            }
        );
#endif
    }
}

