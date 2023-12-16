
#include "user_derived.h"

UserStrategy::UserStrategy(const ThreadConfig* p_cfg) : m_pcfg(p_cfg)
{
    // number of instruments subscribed for data feed
    const auto& gConfig = SYSTEM::get_system_config();
    size_t n = gConfig.inst_list.size();
    m_vIdx.reserve(n);

    // number of instruments bound to this strategy
    size_t m = m_pcfg->inst_list.size();
    m_vBuffer.reserve(m);

    for (size_t i = 0; i < n; ++i)
    {
        m_vIdx[i] = -1; // default for unbound instruments
        for (size_t j = 0; j < m; ++j)
        {
            if (m_pcfg->inst_list[j] == gConfig.inst_list[i])
            {
                m_vIdx[i] = j; // index mapping

                // initialize local buffer
                auto& buffer = m_vBuffer[j];
                memset(&buffer, 0, sizeof(USER_BUFFER));

                break;
            }
        }
    }
}

UserStrategy::~UserStrategy()
{
}


int UserStrategy::on_new_md(int inst_id)
{
    // Unrelevant inst_id returns -1 as default
    return m_vIdx[inst_id];
}

uint16_t UserStrategy::allocate_size(int inst_idx)
{
    /*
     * Whether to acquire output, and 
     * if so, the size of output
     */
    if (m_pcfg->allocate_output[inst_idx]) {
        return sizeof(USER_MSG);
    } else {
        return 0;
    }
}

void UserStrategy::on_new_event(int inst_idx, const MdFeed* in, uint16_t* msg_type, SIGNAL* out)
{
    auto& buffer = m_vBuffer[inst_idx];

    /*
     *  User should consider branching out the compute paths (using strategy cfg)
     *  1. Not all insts need computing signal (runtime turn-off or otherwise configured)
     *  2. Not all insts compute the same factors or signal
     */
    int signal = (in->bid > buffer.bid) ? 1 : 0;

    /*
     *  Make output message if not nullptr
     */
    if (out) { 
        auto* data = static_cast<USER_MSG*>(out);
        data->inst_id = in->inst_id;
        data->signal = signal;
    } 

    /*
     *  After making output message, return ASAP
     */

    // locally buffer whatever desired (those cannot be delayed)
    memcpy(&buffer, in, sizeof(MdFeed));

    // append a delayed task (those can be delayed)
    delayed_queue.emplace(&buffer);
}

void UserStrategy::on_execute(uint16_t msg_type, const SIGNAL* in)
{
    const auto* data = static_cast<const USER_MSG*>(in);
    int inst_id = data->inst_id;
    int signal = data->signal;

    /*
     * Execute w/ TRADER API
     */

    // send buy order at price=100
    uint32_t orderid = TRADER::send_order<'b'>(
        inst_id, 100.0, 1, 0, (char)ORDER_OFFSET::O_OPEN, (void*)this
    );

    // loop over outstanding sell orders of inst_id
    // cancel sell orders if price=100 and status=queueing
    // accumulate the number of cancel
    int num_cancel = 0;
    TRADER::handle_outstanding_orders<'s'>(
        inst_id, 
        [&](const Order& order){
            if (10000 == std::nearbyint(order.price * 100) and
                (char)ORDER_STATUS::O_QUEUEING == order.status) 
            {
                TRADER::cancel_order(order.orderid);
                ++num_cancel;
            }
        }
    );

    /*
     * LOG if needed
     */
    LOG(inst_id, signal);
}


int64_t UserStrategy::on_delayed_event(int64_t tic)
{
    int64_t toc = TIMER::ns();
    if (toc - TIMER::tsc2ns(tic) > 5000000) // 5ms since last event
    {
        if (!delayed_queue.empty()) {
            auto* p_buffer = delayed_queue.front();
            // do delayed task ...
            delayed_queue.pop();
        }
    }
    return TIMER::tsc();
}

void UserStrategy::on_send_err_rtn(Order* p, int errid)
{
    /*
     * Handle any on_send_err
     */
}

void UserStrategy::on_cancel_rtn(Order* p, int cancel_size)
{
    /*
     * Handle any successful cancel
     */
}

void UserStrategy::on_trade_rtn(Order* p, double price, int trade_size)
{
    /*
     * Handle any successful trade
     */
}

void UserStrategy::LOG(int inst_id, int signal)
{
    /*
     *  Fmtlog uses fmt library for formatting
     *
     *  NOTE: 
     *  This is a global logging system. 
     *  Free to log anywhere any content.
     *  File is located at log/[tradedate]/user.log
     */
    int64_t ns = TIMER::ns();
    logi("{},{},{}", ns, inst_id, signal);
}
