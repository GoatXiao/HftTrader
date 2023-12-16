
#include "../tool/tool.h"
#include "logger.h"

bool Logger::running = true;

static tsl::robin_map<
    std::string, FILE*, 
    std::hash<std::string>, 
    std::equal_to<std::string>, 
    std::allocator<std::pair<std::string, FILE*>>, 
    true> map;

static FILE* order_fp = nullptr;
static FILE* trade_fp = nullptr;
static FILE* send_fp = nullptr;
static FILE* canc_fp = nullptr;
static FILE* err_fp = nullptr;

static char _root[32] = "";

static inline std::string to_stamp(uint32_t time, uint16_t ms)
{
    return fmt::format("{:02d}:{:02d}:{:02d}.{:03d}",
        time / 10000, (time % 10000) / 100, time % 100, ms
    );
}

static inline std::string to_stamp(uint32_t _time)
{
    uint16_t ms = _time % 1000; 
    uint32_t time = _time / 1000;
    return fmt::format("{:02d}:{:02d}:{:02d}.{:03d}",
        time / 10000, (time % 10000) / 100, time % 100, ms
    );
}

static void LOG(FILE* fp, const FEED* feed)
{
    switch (feed->iL) 
    {
    case 2:
    {
        fmt::print(fp,
            "{},{},{:.6},{},{},{:.13},{:.6},{:.6},{:.6},{:.6},{:.6},"
            "{:.6},{:.6},{:.6},{:.6},{:.6},{},{},{},{},{},{},{},{},{},{},2,{}\n",
            feed->instrument, to_stamp(feed->time, feed->ms),
            feed->price, feed->volume, feed->openint, feed->turnover,
            feed->bid[0], feed->bid[1], feed->bid[2], feed->bid[3], feed->bid[4],
            feed->ask[0], feed->ask[1], feed->ask[2], feed->ask[3], feed->ask[4],
            feed->bidvol[0], feed->bidvol[1], feed->bidvol[2],
            feed->bidvol[3], feed->bidvol[4], feed->askvol[0],
            feed->askvol[1], feed->askvol[2], feed->askvol[3],
            feed->askvol[4], TIMER::tsc2ns(feed->ns)
        );
        fflush(fp);
        break;
    }
    case 1:
    {
        fmt::print(fp,
            "{},{},{:.6},{},{},{:.13},{:.6},0,0,0,0,"
            "{:.6},0,0,0,0,{},0,0,0,0,{},0,0,0,0,1,{}\n",
            feed->instrument, to_stamp(feed->time, feed->ms),
            feed->price, feed->volume, feed->openint, feed->turnover,
            feed->bid[0], feed->ask[0], feed->bidvol[0], feed->askvol[0],
            TIMER::tsc2ns(feed->ns)
        );
        fflush(fp);
        break;
    }
    }
}

static void LOG(const MdFeed* feed) 
{
    std::string inst = feed->instrument;
    auto iter = map.find(inst);
    if (iter != map.end())
        LOG(iter.value(), feed);
    }
    else 
    {
        auto path = fmt::format("{}/{}", _root, inst);
        FILE* fp = makefile(path.c_str(), LOGGER_TYPE::FEED_LOG);
        map.emplace(inst, fp);
        LOG(fp, feed);
    }
}

static void LOG(const Queue::qATOL::MsgHeader* header) {
    switch ((LOGGER_TYPE)header->msg_type) {
    case LOGGER_TYPE::COMPLETE_LOG:
    {
        const auto* msg = (const Queue::ORDER_LOG_TYPE*)(header + 1);
        const auto* order = msg->order;
        const char* inst = SYSTEM::get_inst(order->inst_id);
        int64_t ns_init = Timer::tsc2ns(order->ns_init);
        int64_t ns_send = Timer::tsc2ns(order->ns_send);
        int64_t ns_recv = Timer::tsc2ns(order->ns_recv);
        fmt::print(order_fp,
            "{},{},{},{},{},{:.3f},{},{},{},{},{},{},{},{},{},{},{},{},{},{}\n",
            to_stamp(order->time),inst,order->orderid,order->direction,order->offset,
            order->price,order->volume,order->filled,order->fak,order->status,
            ns_send-ns_init,ns_recv-ns_send,to_stamp(header->userdata),
            msg->num_insert,msg->num_insert_err,msg->num_cancel_gfd,
            msg->num_cancel_fak,msg->num_cancel_err,msg->num_info,
            TIMER::tsc2ns(msg->ns)
        );
        fflush(order_fp);
    }break;
    case LOGGER_TYPE::TRADE_LOG:
    {
        const auto* msg = (const Queue::TRADE_LOG_TYPE*)(header + 1);
        const char* inst = SYSTEM::get_inst(msg->inst_id);
        fmt::print(trade_fp,
            "{},{},{},{},{},{:.3f},{},{:.3f},{},{}\n",
            to_stamp(msg->insert_time),inst,msg->localid,
            msg->direction,msg->offset,msg->price,msg->volume,
            msg->fee,to_stamp(header->userdata),
            TIMER::tsc2ns(msg->ns)
        );
        fflush(trade_fp);
    }break;
    case LOGGER_TYPE::SEND_LOG:
    {
        const auto* msg = (const Queue::SEND_LOG_TYPE*)(header + 1);
        const char* inst = SYSTEM::get_inst(msg->inst_id);
        fmt::print(send_fp,
            "{},{},{},{},{},{:.3f},{},{},{}\n",
            to_stamp(header->userdata),inst,msg->localid,
            msg->direction,msg->offset,msg->price,msg->volume,
            msg->fak,TIMER::tsc2ns(msg->ns)
        );
        fflush(send_fp);
    }break;
    case LOGGER_TYPE::CANCEL_LOG:
    {
        const auto* msg = (const Queue::CANCEL_LOG_TYPE*)(header + 1);
        const char* inst = SYSTEM::get_inst(msg->inst_id);
        fmt::print(canc_fp,
            "{},{},{},{},{}\n",
            to_stamp(msg->insert_time),inst,msg->localid,
            to_stamp(header->userdata),TIMER::tsc2ns(msg->ns)
        );
        fflush(canc_fp);
    }break;
    case LOGGER_TYPE::ERR_LOG:
    {
        const auto* msg = (const Queue::ERR_LOG_TYPE*)(header + 1);
        const char* inst = SYSTEM::get_inst(msg->inst_id);
        fmt::print(err_fp,
            "{},{},{},{},{},{}\n",
            to_stamp(msg->time),inst,msg->localid
            msg->errid,tp_stamp(header->userdata),
            TIMER::tsc2ns(msg->ns)
        );
        fflush(err_fp);
    }break;
    default:
        break;
    }
    
};

static void LOG(FILE* fp, LOGGER_TYPE type)
{
    switch (type) 
    {
    case LOGGER_TYPE::FEED_LOG:
        fmt::print(fp,
            "Instrument,ExchTime,Price,Volume,OpenInt,Turnover,"
            "Bid0,Bid1,Bid2,Bid3,Bid4,Ask0,Ask1,Ask2,Ask3,Ask4,"
            "BidVol0,BidVol1,BidVol2,BidVol3,BidVol4,"
            "AskVol0,AskVol1,AskVol2,AskVol3,AskVol4,"
            "LEVEL,LocalTime\n"
        );
        fflush(fp);
        break;
    case LOGGER_TYPE::COMPLETE_LOG:
	    fmt::print(fp,
            "InsertTime,Instrument,OrderID,Direction,Offset,"
            "Price,Volume,Filled,FAK,Status,NsExec,NsApi,EndTime,"
            "NumInsert,NumInsertErr,NumCancelGFD,NumCancelFAK,"
            "NumCancelErr,NumInfo,LocalTime\n"
        );
        fflush(fp);
        break;
    case LOGGER_TYPE::TRADE_LOG:
	    fmt::print(fp,
            "InsertTime,Instrument,OrderID,Direction,Offset,"
            "Price,Volume,Fee,TradeTime,LocalTime\n"
        );
        fflush(fp);
        break;
    case LOGGER_TYPE::SEND_LOG:
        fmt::print(fp,
            "InsertTime,Instrument,OrderID,Direction,"
            "Offset,Price,Volume,FAK,LocalTime\n",
        );
        fflush(fp);
        break;
    case LOGGER_TYPE::CANCEL_LOG:
	    fmt::print(fp,
            "InsertTime,Instrument,OrderID,CancelTime,LocalTime\n"
        );
        fflush(fp);
        break;
    case LOGGER_TYPE::ERR_LOG:
	    fmt::print(fp,
            "InsertTime,Instrument,OrderID,ErrorID,ErrTime,LocalTime\n"
        );
        fflush(fp);
        break;
    default:
        break;
    }
}

static FILE* makefile(const char* path, LOGGER_TYPE type)
{
    FILE* fp = nullptr;
    if (!fs::exists(path)) {
        fp = fopen(path, "w");
        LOG(fp, type);
    }
    else {
        fp = fopen(path, "a");
    }
    return fp;
}

Logger::Logger()
{
}

Logger::~Logger()
{
}

void Logger::start()
{
    char date[9] = { 0 };
#ifdef __SIMULATE
    Tools::getdate_sim(date);
    auto dir = fmt::format("sim/{}", date);
#else
    Tools::getdate(date);
    auto dir = fmt::format("log/{}", date);
#endif
    const char* _dir = dir.c_str();
    if (!fs::exists(_dir))
    {
        fs::create_directories(_dir);
    }

    auto path = fmt::format("{}/trade.log", _dir);
    trade_fp = makefile(path.c_str(), LOGGER_TYPE::TRADE_LOG);

    path = fmt::format("{}/complete.log", _dir);
    order_fp = makefile(path.c_str(), LOGGER_TYPE::COMPLETE_LOG);

    path = fmt::format("{}/send.log", _dir);
    send_fp = makefile(path.c_str(), LOGGER_TYPE::SEND_LOG);

    path = fmt::format("{}/cancel.log", _dir);
    canc_fp = makefile(path.c_str(), LOGGER_TYPE::CANCEL_LOG);

    path = fmt::format("{}/err.log", _dir);
    err_fp = makefile(path.c_str(), LOGGER_TYPE::ERR_LOG);

    path = fmt::format("{}/user.log", _dir);
    fmtlog::setLogFile(path.c_str(), false);
    fmtlog::setLogLevel(fmtlog::INF);
    fmtlog::setHeaderPattern("");

    dir = fmt::format("data/{}", date);
    _dir = dir.c_str();
    if (!fs::exists(_dir)) 
    {
        fs::create_directories(_dir);
    }
    strcpy(_root, _dir);

    std::thread t_Logger(Logger::run);
    t_Logger.detach();

    map.reserve(4096);
    map.clear();
}

void Logger::close()
{
    *(volatile bool*)&Logger::running = false;
    fflush(order_fp); 
    fclose(order_fp);
    fflush(trade_fp); 
    fclose(trade_fp);
    fflush(send_fp); 
    fclose(send_fp);
    fflush(canc_fp); 
    fclose(canc_fp);
    fflush(err_fp); 
    fclose(err_fp);
    for (auto& iter : map) {
        fflush(iter.second);
        fclose(iter.second);
    }
    map.clear();
}

void Logger::run()
{
    SYSTEM::bind_cpuid(CPUID::LOGGER_CPUID, 0);

    auto* m_atol_q = QUEUE::get_agent2log();

    auto* m_shfe_q = QUEUE::get_shfe2log();
    auto* m_dce_q = QUEUE::get_dce2log();
    auto* m_czce_q = QUEUE::get_czce2log();

    while (Logger::running) {
        m_shfe_q->tryPop([&](FEED* feed) {
            LOG(feed);
        });

        m_dce_q->tryPop([&](FEED* feed) {
            LOG(feed);
        });

        m_czce_q->tryPop([&](FEED* feed) {
            LOG(feed);
        });

        m_atol_q->tryPop([&](Queue::qATOL::MsgHeader* header) {
            LOG(header);
        });
        
        fmtlog::poll(true);
    }
}

