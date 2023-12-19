
#include "../tool/tool.h"

static TSCNS tscns;

static SystemConfig gConfig;
static std::vector<ThreadConfig> v_tConfig;
static std::vector<InstrumentConfig> v_iConfig;
static HashMap<InstrumentConfig>::type g_InstrumentMap;

static Queue::qFTOL q_shfe2log;
static Queue::qFTOL q_dce2log;
static Queue::qFTOL q_czce2log;

static Queue::qFTOU q_shfe2user;
static Queue::qFTOU q_dce2user;
static Queue::qFTOU q_czce2user;

static std::vector<Queue::qUTOA*> vp_q_user2agent;
static Queue::qCBTOA q_api2agent;
static Queue::qATOL q_agent2log;

static std::vector<MdFeed> v_Feed;


namespace QUEUE
{
    void init() {
        vp_q_user2agent.reserve(v_tConfig.size());
        for (size_t i = 0; i < v_tConfig.size(); ++i) {
            vp_q_user2agent[i] = new SPSCVarQueueOPT<4096>;
        }
    };

    Queue::qFTOL* get_shfe2log() { return &q_shfe2log; };
    Queue::qFTOL* get_dce2log() { return &q_dce2log; };
    Queue::qFTOL* get_czce2log() { return &q_czce2log; };
    
    Queue::qFTOU* get_shfe2user() { return &q_shfe2user; };
    Queue::qFTOU* get_dce2user() { return &q_dce2user; };
    Queue::qFTOU* get_czce2user() { return &q_czce2user; };

    std::vector<Queue::qUTOA*> get_user2agent() { return vp_q_user2agent; }
    Queue::qUTOA* get_user2agent(int i) { return vp_q_user2agent[i]; };
    Queue::qCBTOA* get_api2agent() { return &q_api2agent; };
    Queue::qATOL* get_agent2log() { return &q_agent2log; };
} // namespace QUEUE


namespace SYSTEM
{
    void init_system() {
        Tools::read_system_config();
        Tools::read_order_log();
        Tools::query_ctp();

        g_InstrumentMap.clear();
        for (auto& v : v_iConfig) {
            g_InstrumentMap.emplace(v.inst.data(), &v);
        }
        g_InstrumentMap.doneModify();
        g_InstrumentMap.clear();

        v_Feed.resize(v_iConfig.size());

        TIMER::init();
        QUEUE::init();
    };

    // 设置CPU亲和度/线程策略(非FIFO调度prio=0)
    void bind_cpuid(int cpuid, int priority) {
        cpu_set_t mask; CPU_ZERO(&mask); CPU_SET(cpuid, &mask);
        pthread_setaffinity_np(pthread_self(), sizeof(mask), &mask);
        if (priority > 0) {
            struct sched_param param;
            param.sched_priority = priority;
            pthread_setschedparam(pthread_self(), SCHED_FIFO, &param);
        }
    };

    SystemConfig& get_system_cfg() { return gConfig; };
    char* get_inst(int idx) { return gConfig.inst_list[idx].data(); };

    std::vector<ThreadConfig>& get_thread_cfgs() { return v_tConfig; };
    std::vector<InstrumentConfig>& get_inst_cfgs() { return v_iConfig; };

    InstrumentConfig& get_inst_cfg(int idx) { return v_iConfig[idx]; };
    ThreadConfig& get_thread_cfg(int idx) { return v_tConfig[idx]; };

    InstrumentConfig* find_inst_cfg(const char* inst) 
    { 
        return g_InstrumentMap.fastFind(inst);
    };

    // libconfig API
    bool read_cfg_file(Config& cfg, const char* fn) 
    { 
        return Tools::read_cfg_file(cfg, fn); 
    };

    Setting& get_cfg_root(const Config& cfg) 
    {
        return Tools::get_cfg_root(cfg);
    };

    int get_cfg_length(const Setting& cfg) 
    {
        return Tools::get_cfg_length(cfg);
    };

    Setting& get_cfg_member(const Setting& cfg, const char* path) 
    {
        return Tools::get_cfg_member(cfg, path);
    };

    void get_cfg_item_int(const Setting& cfg, const char* path, int& item)
    {
        Tools::get_cfg_item<int>(cfg, path, item);
    }

    void get_cfg_item_double(const Setting& cfg, const char* path, double& item)
    {
        Tools::get_cfg_item<double>(cfg, path, item);
    }

    void get_cfg_item_string(const Setting& cfg, const char* path, std::string& item)
    {
        Tools::get_cfg_item_string(cfg, path, item);
    }

    void get_cfg_array_int(const Setting& cfg, const char* path, std::vector<int>& vec)
    {
        Tools::get_cfg_array<int>(cfg, path, vec);
    }

    void get_cfg_array_double(const Setting& cfg, const char* path, std::vector<double>& vec)
    {
        Tools::get_cfg_array<double>(cfg, path, vec);
    }

    void get_cfg_array_string(const Setting& cfg, const char* path, std::vector<std::string>& vec)
    {
        Tools::get_cfg_array_string(cfg, path, vec);
    }

    // Terminal Console
    void reload_instrument_config(const char* inst) 
    {
        auto* p_cfg = SYSTEM::find_inst_cfg(inst);
        if (p_cfg) { Tools::reload_instrument_config(p_cfg); }
    };

    void reload_thread_config(int thread_id) 
    {
        if (thread_id < gConfig.num_strategy_threads) {
            Tools::reload_thread_config(thread_id);
        }
    };

} // namespace SYSTEM


namespace TIMER
{
    void init() { tscns.init(TSCNS::NsPerSec); };
    int64_t tsc2ns(int64_t tsc) { return tscns.tsc2ns(tsc); };
    int64_t tsc() { return tscns.rdtsc(); };
    int64_t ns() { return tscns.rdns(); };
} // namespace TIMER
    
    
namespace FEED
{
    uint32_t get_timestamp(const MdFeed* md) { 
        return md->time * 1000 + md->ms; 
    };
   
    void set(const MdFeed* feed) {
        int inst_id = feed->p_cfg->inst_id;
        memcpy(&v_Feed[inst_id], feed, sizeof(MdFeed));
    };

    MdFeed* get(int inst_id) {
        return &v_Feed[inst_id];
    };
} // namespace FEED
