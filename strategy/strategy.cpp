#include "strategy.h"
#include "../user.h"

static int64_t process(const MdFeed* in, Queue::qUTOA* out, UserStrategyBase* pUser)
{
    InstrumentConfig* p_cfg = in->p_cfg;
    int inst_idx = pUser->on_new_md(p_cfg->inst_id);
    if (inst_idx == -1) { return; }

    uint16_t size = pUser->allocate_size(inst_idx);
    if (size > 0) 
    {
        Queue::qUTOA::MsgHeader* header = nullptr;
        do { header = out->alloc(size); } while (!header);
        auto* output = (UserStrategyBase::SIGNAL*)(header + 1);
        header->userdata = FEED::get_timestamp(in);
        output->p_user = pUser;
        output->p_cfg = p_cfg;
        output->ns_data = in->ns;
        output->bid = in->bid[0];
        output->ask = in->ask[0];
        output->bidvol = in->bidvol[0];
        output->askvol = in->askvol[0];
        pUser->on_new_event(inst_idx, in, &header->msg_type, output);
        output->ns_done = Timer::tsc();
        out->push();
    }
    else 
    {
        pUser->on_new_event(inst_idx, in, nullptr, nullptr);
    }
    return TIMER::tsc();
}

void Strategy::run(Strategy* pStrategy)
{
    const auto& thread_cfg = SYSTEM::get_thread_cfg(pStrategy->m_thread_id);
    SYSTEM::bind_cpuid(thread_cfg.cpu_id, THREAD_PRIORITY::THREAD_USER);
    UserStrategyBase* pUserStrategy = USER::getUserStrategy(thread_cfg);

    auto* q_out = QUEUE::get_user2agent(pStrategy->m_thread_id);

    if (pUserStrategy && q_out)
    {
        auto* q_shfe = QUEUE::get_shfe2user();
        auto* q_dce = QUEUE::get_dce2user();
        auto* q_czce = QUEUE::get_czce2user();

        auto reader_shfe = q_shfe->getReader();
        auto reader_dce = q_dce->getReader();
        auto reader_czce = q_czce->getReader();

        MdFeed* input = nullptr;
        int64_t ns_done = 0;

        while (pStrategy->running)
        {
            input = reader_shfe.read();
            if (input) {
                ns_done = process(input, q_out, pUserStrategy);
                input = nullptr;
            }

            input = reader_dce.read();
            if (input) {
                ns_done = process(input, q_out, pUserStrategy);
                input = nullptr;
            }
            
            input = reader_czce.read();
            if (input) {
                ns_done = process(input, q_out, pUserStrategy);
                input = nullptr;
            }

            ns_done = pUserStrategy->on_delayed_event(ns_done);
        }
    
        delete pUserStrategy;
        pUserStrategy = nullptr;
    }
}

void Strategy::start()
{
    std::thread t_Strategy(Strategy::run, this);
    t_Strategy.detach();
}

void Strategy::close()
{
    *(volatile bool*)&running = false;
}

Strategy::Strategy(int thread_id)
    : m_thread_id(thread_id), running(true)
{
}

Strategy::~Strategy()
{
}
