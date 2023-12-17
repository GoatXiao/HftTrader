
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
        for (int i = 0; i < v_tConfig.size(); ++i) {
            vp_q_user2agent[i] = new SPSCVarQueueOPT<4096>;
        }
    };

    inline Queue::qFTOL* get_shfe2log() { return &q_shfe2log; };
    inline Queue::qFTOL* get_dce2log() { return &q_dce2log; };
    inline Queue::qFTOL* get_czce2log() { return &q_czce2log; };
    
    inline Queue::qFTOU* get_shfe2user() { return &q_shfe2user; };
    inline Queue::qFTOU* get_dce2user() { return &q_dce2user; };
    inline Queue::qFTOU* get_czce2user() { return &q_czce2user; };

    inline std::vector<Queue::qUTOA*> get_user2agent() { return vp_q_user2agent; }
    inline Queue::qUTOA* get_user2agent(int i) { return vp_q_user2agent[i]; };
    inline Queue::qCBTOA* get_api2agent() { return &q_api2agent; };
    inline Queue::qATOL* get_agent2log() { return &q_agent2log; };
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

    inline SystemConfig& get_system_cfg() { return gConfig; };
    inline char* get_inst(int idx) { return gConfig.inst_list[idx].data(); };

    inline std::vector<ThreadConfig>& get_thread_cfgs() { return v_tConfig; };
    inline std::vector<InstrumentConfig>& get_inst_cfgs() { return v_iConfig; };

    inline InstrumentConfig& get_inst_cfg(int idx) { return v_iConfig[idx]; };
    inline ThreadConfig& get_thread_cfg(int idx) { return v_tConfig[idx]; };

    inline InstrumentConfig* find_inst_cfg(const char* inst) 
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

    template<typename T, bool str_type=false>
    void get_cfg_item(const Setting& cfg, const char* path, T& item) 
    {
        Tools::get_cfg_item<T, str_type>(cfg, path, item);
    };

    template<typename T, bool str_type=false>
    void get_cfg_array(const Setting& cfg, const char* path, std::vector<T>& vec) 
    {
        Tools::get_cfg_array(cfg, path, vec);
    };

    // Terminal Console
    void reload_instrument_config(const char* inst) 
    {
        volatile auto* p_cfg = SYSTEM::find_inst_cfg(inst);
        if (p) { Tools::reload_instrument_config(p); }
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
    inline int64_t tsc2ns(int64_t tsc) { return tscns.tsc2ns(tsc); };
    inline int64_t tsc() { return tscns.rdtsc(); };
    inline int64_t ns() { return tscns.rdns(); };
} // namespace TIMER
    
    
namespace FEED
{
    inline uint32_t get_timestamp(const MdFeed* md) { 
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
