

#include "feed_dce.h"
#include "../../system/system.h"

static void set_timestamp(uint32_t* time, uint16_t* ms, uint64_t _s) {
    uint32_t t = _s / 1000000000;
    int s = t % 60;
    int m = (t % 3600) / 60;
    int h = (t / 3600) % 24 + 8;
    *ms = _s / 1000000 - t * 1000;
    *time = h * 10000 + m * 100 + s;
}

static void set_timestamp(uint32_t* time, uint16_t* ms, const char* _s) {
    int h = std::atoi(_s);
    int m = std::atoi(_s + 3);
    int s = std::atoi(_s + 6);
    *ms = std::atoi(_s + 9);
    *time = h * 10000 + m * 100 + s;
};

#define __DCE_NANO 0

#if __DCE_NANO
#include "NanoDceMdApi.h"
class CNanoDceMdSpiImpl : public CNanoDceMdSpi
{
private:
    struct alignas(8) LAST {
        LAST(double t, double p, int o, int v) 
            : turnover(t), price(p), openint(o), volume(v) 
        { 
        };

        double turnover{0};
        double price{0};
        int openint{0};
        int volume{0};
    };

    Queue::QFTOU* q2u = nullptr;
    Queue::QFTOL* q2l = nullptr;
    HashMap<LAST>::type last;

public:
    CNanoDceMdSpiImpl() {
        q2u = QUEUE::get_dce2user();
        q2l = QUEUE::get_dce2log();
        last.doneModify();
    };
    ~CNanoDceMdSpiImpl() { };

public:
    //一档行情回调接口
    virtual void OnNanoDceL1Md(const NanoDceL1MdType& md) {
        int64_t ns = TIMER::tsc();

        uint32_t time; 
        uint16_t ms;
        set_timestamp(&time, &ms, md.send_time);

	    double bid_price = (double)md.bid_price / 10000;
	    double ask_price = (double)md.ask_price / 10000;
	    double last_price = (double)md.last_price / 10000;
	    double turnover = (double)md.turn_over / 10000;

        const char* inst = (const char*)md.contract_name;
        auto* p_cfg = SYSTEM::find_inst_cfg(inst);
        if (p_cfg)
        {
            q2u->blockPush([&](MdFeed* feed) {
                memcpy(feed->instrument, inst, INSTRUMENTLENGTH);
                feed->p_cfg = p_cfg;
                feed->bid[0] = bid_price;
                feed->ask[0] = ask_price;
                feed->turnover = turnover;
                feed->price = last_price;
                feed->openint = md.open_interest;
                feed->bidvol[0] = md.bid_qty;
                feed->askvol[0] = md.ask_qty;
                feed->volume = md.total_qty;
                feed->time = time;
                feed->ms = ms;
                feed->ns = ns;
                feed->iL = 1;
            });
        }
        q2l->blockPush([&](MdFeed* feed) {
            memcpy(feed->instrument, inst, INSTRUMENTLENGTH);
            feed->bid[0] = bid_price;
            feed->ask[0] = ask_price;
            feed->turnover = turnover;
            feed->price = last_price;
            feed->openint = md.open_interest;
            feed->bidvol[0] = md.bid_qty;
            feed->askvol[0] = md.ask_qty;
            feed->volume = md.total_qty;
            feed->time = time;
            feed->ms = ms;
            feed->ns = ns;
            feed->iL = 1;
        });
    };

    //五档行情回调接口
    virtual void OnNanoDceL2ContractBestPriceMd(const NanoDceL2ContractBestPriceMdType& md) {
        int64_t ns = TIMER::tsc();

        uint32_t time;
        uint16_t ms;
        set_timestamp(&time, &ms, (const char*)md.gen_time);

        const char* inst = (const char*)md.contract_id;
        auto* ptr = last.fastFind(inst);
        if (ptr) 
        {
            auto* p_cfg = SYSTEM::find_inst_cfg(inst);
            if (p_cfg) 
            {
                q2u->blockPush([&](MdFeed* feed) {
                    memcpy(feed->instrument, inst, INSTRUMENTLENGTH);
                    feed->p_cfg = p_cfg;
                    feed->price = md.last_price;
                    feed->turnover = md.turnover;
                    feed->openint = md.open_interest;
                    feed->volume = md.match_tot_qty;
                    feed->bidvol[0] = md.bid_qty;
                    feed->askvol[0] = md.ask_qty;
                    feed->bid[0] = md.bid_price;
                    feed->ask[0] = md.ask_price;
                    feed->time = time;
                    feed->ms = ms;
                    feed->ns = ns;
                    feed->iL = 1;
                });
            }
            q2l->blockPush([&](MdFeed* feed) {
                memcpy(feed->instrument, inst, INSTRUMENTLENGTH);
                feed->price = md.last_price;
                feed->turnover = md.turnover;
                feed->openint = md.open_interest;
                feed->volume = md.match_tot_qty;
                feed->bidvol[0] = md.bid_qty;
                feed->askvol[0] = md.ask_qty;
                feed->bid[0] = md.bid_price;
                feed->ask[0] = md.ask_price;
                feed->time = time;
                feed->ms = ms;
                feed->ns = ns;
                feed->iL = 1;
            });
            ptr->price = md.last_price;
            ptr->turnover = md.turnover;
            ptr->openint = md.open_interest;
            ptr->volume = md.match_tot_qty;
        } 
        else 
        {
            ptr = new LAST(
                md.turnover,
                md.last_price,
                md.open_interest,
                md.match_tot_qty
            );
            last.emplace(inst, ptr);
            last.doneModify();
        }
    };
    virtual void OnNanoDceL2ArbBestPriceMd(const NanoDceL2ArbBestPriceMdType& refNanoDceL2ArbBestPriceMd) {};
    virtual void OnNanoDceL2SegQuotaMd(const NanoDceL2SegQuotaMdType& refNanoDceL2SegQuotaMd) {};
    virtual void OnNanoDceL2OrderStatisticsMd(const NanoDceL2OrderStatisticsMdType& refNanoDceL2OrderStatisticsMd) {};
    virtual void OnNanoDceL2DeepOrderVolumeMd(const NanoDceL2DeepOrderVolumeMdType& refDeepOrderVolumeMd) {};
    virtual void OnNanoDceL2DeepQuoteMd(const NanoDceL2DeepQuoteMdType& md) {
        int64_t ns = TIMER::tsc();
        
        uint32_t time;
        uint16_t ms;
        set_timestamp(&time, &ms, (const char*)md.gen_time);

        const char* inst = (const char*)md.contract_id;
        const auto* ptr = last.fastFind(inst);
        if (ptr) 
        {
            auto* p_cfg = SYSTEM::find_inst_cfg(inst);
            if (p_cfg) 
            {
                q2u->blockPush([&](FEED* feed) {
                    memcpy(feed->instrument, inst, INSTRUMENTLENGTH);
                    feed->p_cfg = p_cfg;
                    feed->price = ptr->price;
                    feed->turnover = ptr->turnover;
                    feed->openint = ptr->openint;
                    feed->volume = ptr->volume;
                    feed->bidvol[0] = md.bid_1_qty;
                    feed->bidvol[1] = md.bid_2_qty;
                    feed->bidvol[2] = md.bid_3_qty;
                    feed->bidvol[3] = md.bid_4_qty;
                    feed->bidvol[4] = md.bid_5_qty;
                    feed->askvol[0] = md.ask_1_qty;
                    feed->askvol[1] = md.ask_2_qty;
                    feed->askvol[2] = md.ask_3_qty;
                    feed->askvol[3] = md.ask_4_qty;
                    feed->askvol[4] = md.ask_5_qty;
                    feed->bid[0] = md.bid_1;
                    feed->bid[1] = md.bid_2;
                    feed->bid[2] = md.bid_3;
                    feed->bid[3] = md.bid_4;
                    feed->bid[4] = md.bid_5;
                    feed->ask[0] = md.ask_1;
                    feed->ask[1] = md.ask_2;
                    feed->ask[2] = md.ask_3;
                    feed->ask[3] = md.ask_4;
                    feed->ask[4] = md.ask_5;
                    feed->time = time;
                    feed->ms = ms;
                    feed->ns = ns;
                    feed->iL = 2;
                });
            }
            q2l->blockPush([&](FEED* feed) {
                memcpy(feed->instrument, inst, INSTRUMENTLENGTH);
                feed->price = ptr->price;
                feed->turnover = ptr->turnover;
                feed->openint = ptr->openint;
                feed->volume = ptr->volume;
                feed->bidvol[0] = md.bid_1_qty;
                feed->bidvol[1] = md.bid_2_qty;
                feed->bidvol[2] = md.bid_3_qty;
                feed->bidvol[3] = md.bid_4_qty;
                feed->bidvol[4] = md.bid_5_qty;
                feed->askvol[0] = md.ask_1_qty;
                feed->askvol[1] = md.ask_2_qty;
                feed->askvol[2] = md.ask_3_qty;
                feed->askvol[3] = md.ask_4_qty;
                feed->askvol[4] = md.ask_5_qty;
                feed->bid[0] = md.bid_1;
                feed->bid[1] = md.bid_2;
                feed->bid[2] = md.bid_3;
                feed->bid[3] = md.bid_4;
                feed->bid[4] = md.bid_5;
                feed->ask[0] = md.ask_1;
                feed->ask[1] = md.ask_2;
                feed->ask[2] = md.ask_3;
                feed->ask[3] = md.ask_4;
                feed->ask[4] = md.ask_5;
                feed->time = time;
                feed->ms = ms;
                feed->ns = ns;
                feed->iL = 2;
            });
        }
    };
};
#endif

bool Feed_DCE::running = true;

void Feed_DCE::start()
{
    std::thread t_recv(Feed_DCE::run);
    t_recv.detach();
}

void Feed_DCE::close()
{
    *(volatile bool*)&running = false;
}

void Feed_DCE::run()
{
    SYSTEM::bind_cpuid(CPUID::FEED_DCE_CPUID, 0);
#if __DCE_NANO
    CNanoDceMdSpiImpl mdSpi;
    fmt::print("Launch NanoDceSpi\n");
    auto& MdApi = CNanoDceMdApi::CreateNanoDceMdApi();
    MdApi.NanoRecvStart(mdSpi, "nano.ini");
#endif
}
