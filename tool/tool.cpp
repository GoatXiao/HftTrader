
#include "../api/ctp2mini/ctp2miniOSpi.h"
#include "tool.h"

char Tools::m_date[DATE_TIME_LEN] = { 0 };
std::atomic<bool> Tools::simulate_lock = { true };

Tools::Tools()
{
}

Tools::~Tools()
{
}

void Tools::setdate(char* _date)
{
    memcpy(m_date, _date, DATE_TIME_LEN);
}

void Tools::getdate_sim(char* _date)
{
    if (8 != strlen(m_date)) 
    {
        memset(m_date, 0, DATE_TIME_LEN);
        getdate(m_date);
    }
    memcpy(_date, m_date, DATE_TIME_LEN);
}

void Tools::unlock_sim() {
    simulate_lock.store(false, std::memory_order_release);
}

void Tools::lock_sim() {
    simulate_lock.store(true, std::memory_order_release);
}

bool Tools::is_lock() {
    return simulate_lock.load(std::memory_order_acquire);
}

void Tools::getdate(char* _date)
{
    time_t now = time(0);
    tm* ltm = localtime(&now);

    int y, m, d;
    y = 1900 + ltm->tm_year;
    m = 1 + ltm->tm_mon;
    d = ltm->tm_mday;

    char t[DATE_TIME_LEN] = { 0 };
    snprintf(t, DATE_TIME_LEN, "%02d:%02d:%02d", ltm->tm_hour, ltm->tm_min, ltm->tm_sec);
    if (strncmp(t, "08:00:00", sizeof(t)) >= 0 && strncmp(t, "16:00:00", sizeof(t)) <= 0)
    {
        snprintf(_date, DATE_TIME_LEN, "%04d%02d%02d", y, m, d);
    }
    else
    {
        if (ltm->tm_wday == 5)////周五 跳过3天
        {
            char curdate[DATE_TIME_LEN] = { 0 };
            snprintf(curdate, DATE_TIME_LEN, "%04d%02d%02d", y, m, d);
            for (int i = 0; i < 3; i++)
            {
                int _y = 0, _m = 0, _d = 0;
                explodeday(curdate, _y, _m, _d);
                getnextday(_date, _y, _m, _d);
                memcpy(curdate, _date, sizeof(curdate));
            }
        }
        else
        {
            getnextday(_date, y, m, d);
        }
    }
}

void Tools::explodeday(const char* date, int& _y, int& _m, int& _d)
{
    char y[5] = { 0 };
    char m[3] = { 0 };
    char d[3] = { 0 };

    memcpy(y, date, 4);
    memcpy(m, date + 4, 2);
    memcpy(d, date + 6, 2);
    _y = atoi(y);
    _m = atoi(m);
    _d = atoi(d);
}

void Tools::getnextday(char* date, const int& _y, const int& _m, const int& _d)
{
    int y = _y;
    int m = _m;
    int d = _d;
    if ((m == 1 || 3 || 5 || 7 || 8 || 10) && (d == 31))
    {
        m++;
        d = 1;
    }
    else if ((m == 4 || 6 || 9 || 11) && (d == 30))
    {
        m++;
        d = 1;
    }
    else if ((m == 2) && (d == 28))
    {
        if (((y % 4 == 0) && (y % 100 != 0)) || ((y % 400) == 0))//判断闰年
        {
            d++;
        }
        else
        {
            m++;
            d = 1;
        }
    }
    else if ((m == 2) && (d == 29))
    {
        m++;
        d = 1;
    }
    else if ((m == 12) && (d == 31))
    {
        y++;
        m = 1;
        d = 1;
    }
    else
    {
        d++;
    }
    snprintf(date, DATE_TIME_LEN, "%04d%02d%02d", y, m, d);
}


/*
 * read libconfig api
 */
bool Tools::read_cfg_file(Config& cfg, const char* filePath)
{
    try
    {
        cfg.readFile(filePath);
    }
    catch (const FileIOException& fioex)
    {
        fmt::print("I/O error while reading file {}\n", filePath);
        return false;
    }
    catch (const ParseException& pex)
    {
        fmt::print("Parse error at {}:{}-{} \n", pex.getFile(), pex.getLine(), pex.getError());
        return false;
    }
    return true;
}

Setting& Tools::get_cfg_root(const Config& cfg)
{
    return cfg.getRoot();
}

int Tools::get_cfg_length(const Setting& cfg)
{
    return cfg.getLength();
}

Setting& Tools::get_cfg_member(const Setting& cfg, const char* path)
{
    try
    {
        return cfg.lookup(path);
    }
    catch (const SettingNotFoundException& nfex)
    {
        fmt::print("Lookup {} Failed\n", path);
        return const_cast<Setting&>(cfg);
    }
}

template<typename T>
void Tools::get_cfg_item(const Setting& cfg, const char* path, T& out)
{
    try
    {
        Setting& v = cfg.lookup(path);
        out = v;
    }
    catch (const SettingNotFoundException& nfex)
    {
        fmt::print("Lookup {} Failed\n", path);
    }
}

void Tools::get_cfg_item_string(const Setting& cfg, const char* path, std::string& out)
{
    try
    {
        Setting& v = cfg.lookup(path);
        out = (const char*)v;
    }
    catch (const SettingNotFoundException& nfex)
    {
        fmt::print("Lookup {} Failed\n", path);
    }
}

template<typename T>
void Tools::get_cfg_array(const Setting& cfg, const char* path, std::vector<T>& out)
{
    try
    {
        Setting& arr = cfg.lookup(path);
        int n = get_cfg_length(arr);
        out.reserve(n);
        for (int i = 0; i < n; ++i) {
            out[i] = arr[i];
        }
    }
    catch (const SettingNotFoundException& nfex)
    {
        fmt::print("Lookup {} Failed\n", path);
    }
}

void Tools::get_cfg_array_string(const Setting& cfg, const char* path, std::vector<std::string>& out)
{
    try
    {
        Setting& arr = cfg.lookup(path);
        int n = get_cfg_length(arr);
        out.reserve(n);
        for (int i = 0; i < n; ++i) {
            out[i] = (const char*)arr[i];
        }
    }
    catch (const SettingNotFoundException& nfex)
    {
        fmt::print("Lookup {} Failed\n", path);
    }
}

void Tools::read_system_config()
{
    Config cfg;
    if (!read_cfg_file(cfg, "config/system.cfg")) {
        return;
    }
    const auto& root = get_cfg_root(cfg);

    auto& gConfig = SYSTEM::get_system_cfg();

    // user cfg
    const auto& user_cfgs = get_cfg_member(root, "config.user");
    int num_users = get_cfg_length(user_cfgs); // 目前只支持num_users = 1
    fmt::print("[system] num_users = {}\n", num_users);
    for (int i = 0; i < num_users; ++i) 
    {
        get_cfg_item_string(user_cfgs[i], "BrokerId", gConfig.BrokerId);
        get_cfg_item_string(user_cfgs[i], "UserId", gConfig.UserId);
        get_cfg_item_string(user_cfgs[i], "PassWord", gConfig.PassWord);
        get_cfg_item_string(user_cfgs[i], "AppId", gConfig.AppId);
        get_cfg_item_string(user_cfgs[i], "AuthCode", gConfig.AuthCode);
        fmt::print("[system] BrokerId={},UserId={},PassWord={},AppId={},AuthCode={}\n",
            gConfig.BrokerId.data(), gConfig.UserId.data(),
            gConfig.PassWord.data(), gConfig.AppId.data(),
            gConfig.AuthCode.data()
        );
    }

    // market md cfg
    const auto& md_cfg = get_cfg_member(root, "config.md");
    get_cfg_item_string(md_cfg, "Interface", gConfig.MdInterface);
    get_cfg_item_string(md_cfg, "LocalAddr", gConfig.MdLocalAddr);
    get_cfg_item_string(md_cfg, "RemoteAddr", gConfig.MdRemoteAddr);
    get_cfg_item<int>(md_cfg, "RemotePort", gConfig.MdRemotePort);
    fmt::print("[system] MdInterface={},MdLocalAddr={},MdRemoteAddr={}:{}\n",
        gConfig.MdInterface.data(), gConfig.MdLocalAddr.data(),
        gConfig.MdRemoteAddr.data(), gConfig.MdRemotePort
    );

    // trade cfg
    const auto& td_cfg = get_cfg_member(root, "config.trade");
    get_cfg_item_string(td_cfg, "Interface", gConfig.TradeInterface);
    get_cfg_item_string(td_cfg, "LocalAddr", gConfig.TradeLocalAddr);
    get_cfg_item_string(td_cfg, "TradeAddr", gConfig.TradeAddr);
    get_cfg_item_string(td_cfg, "QueryAddr", gConfig.QueryAddr);
    get_cfg_item<int>(td_cfg, "TradePort", gConfig.TradePort);
    get_cfg_item<int>(td_cfg, "QueryPort", gConfig.QueryPort);
    fmt::print("[system] Interface={},LocalAddr={},TradeAddr={}:{},QueryAddr={}:{}\n",
        gConfig.TradeInterface.data(), gConfig.TradeLocalAddr.data(),
        gConfig.TradeAddr.data(), gConfig.TradePort,
        gConfig.QueryAddr.data(), gConfig.QueryPort
    );

    // broker cfg
    const auto& broker_cfg = get_cfg_member(root, "config.broker");
    get_cfg_item<double>(broker_cfg, "fee_per_volume", gConfig.fee_per_volume);
    get_cfg_item<double>(broker_cfg, "rebate_rate", gConfig.rebate_rate);
    fmt::print("[system] broker_fee_per_volume={:.2},broker_rebate_rate={:.2}\n",
        gConfig.fee_per_volume, gConfig.rebate_rate
    );

    // simulate cfg
    const auto& sim_cfg = get_cfg_member(root, "config.simulate");
    get_cfg_item<double>(sim_cfg, "hit_rate", gConfig.hit_rate);
    get_cfg_item<double>(sim_cfg, "place_rate", gConfig.place_rate);
    fmt::print("[system] sim_hit_rate={:.2},sim_place_rate={:.2}\n",
        gConfig.hit_rate, gConfig.place_rate
    );

    // strategy cfg
    const auto& strategy_cfg = get_cfg_member(root, "config.strategy");
    get_cfg_array_string(strategy_cfg, "subscription", gConfig.inst_list);
    get_cfg_item<int>(strategy_cfg, "num_threads", gConfig.num_strategy_threads);
    fmt::print("[system] num_strategy_threads={}\n", gConfig.num_strategy_threads);

    // thread cfg
    const auto& thread_cfgs = get_cfg_member(strategy_cfg, "thread");
    auto& v_tConfig = SYSTEM::get_thread_cfgs();
    v_tConfig.clear();

    for (int i = 0; i < gConfig.num_strategy_threads; ++i)
    {
        ThreadConfig thread_cfg;

        get_cfg_array_string(thread_cfgs[i], "inst_list", thread_cfg.inst_list);
        get_cfg_array<int>(thread_cfgs[i], "allocate_output_msg", thread_cfg.allocate_output);
        get_cfg_item<int>(thread_cfgs[i], "strategy_id", thread_cfg.strategy_id);
        get_cfg_item<int>(thread_cfgs[i], "bind_cpuid", thread_cfg.cpu_id);

        fmt::print("[thread {}] bind_cpuid={},strategy_id={},inst:alloc=[ ", 
            i, thread_cfg.cpu_id, thread_cfg.strategy_id
        );

        for (size_t j = 0; j < thread_cfg.inst_list.size(); ++j) {
            fmt::print("{}:{} ", 
                thread_cfg.inst_list[j].data(), 
                thread_cfg.allocate_output[j]
            );
        }
        fmt::print("]\n");

        v_tConfig.push_back(thread_cfg);
    }
    v_tConfig.shrink_to_fit();

    // instrument cfg
    fmt::print("[system] subscription: ");
    std::map<std::string, int> instid_map; // inst -> inst_id
    for (size_t i = 0; i < gConfig.inst_list.size(); ++i) {
        instid_map.emplace(gConfig.inst_list[i], i);
        fmt::print("{} ", gConfig.inst_list[i].data());
    }
    fmt::print("\n");

    const auto& inst_cfgs = get_cfg_member(strategy_cfg, "instrument");
    auto& v_iConfig = SYSTEM::get_inst_cfgs();
    v_iConfig.clear();

    for (size_t i = 0; i < gConfig.inst_list.size(); ++i)
    {
        InstrumentConfig inst_cfg;
        inst_cfg.inst_id = instid_map[gConfig.inst_list[i]];
        get_cfg_item_string(inst_cfgs[i], "inst", inst_cfg.inst);
        get_cfg_item_string(inst_cfgs[i], "param_path", inst_cfg.param_path);
        get_cfg_item<double>(inst_cfgs[i], "exchange_rebate_rate", inst_cfg.exchange_rebate_rate);
        get_cfg_item<int>(inst_cfgs[i], "offset_type_mode", inst_cfg.offset_type_mode);
        get_cfg_item<int>(inst_cfgs[i], "max_inventory", inst_cfg.max_inventory);
        get_cfg_item<int>(inst_cfgs[i], "max_net_pos", inst_cfg.max_net_pos);
        get_cfg_item<int>(inst_cfgs[i], "max_volume", inst_cfg.max_volume);
        get_cfg_item<int>(inst_cfgs[i], "max_cancel", inst_cfg.max_cancel);
        
        get_cfg_array_string(inst_cfgs[i], "inst_list", inst_cfg.inst_list);
        inst_cfg.num_inst = inst_cfg.inst_list.size();
 
        inst_cfg.final_rebate_rate = inst_cfg.exchange_rebate_rate * gConfig.rebate_rate;

        fmt::print(
            "[instrument {}] inst={},param_path={},final_rebate_rate={:.2},"
            "offset_type_mode={},max_inventory={},max_volume={},max_cancel={},"
            "max_net_pos={},inst_list(N={})=[ ",
            inst_cfg.inst_id,inst_cfg.inst.data(),inst_cfg.param_path.data(),
            inst_cfg.final_rebate_rate,inst_cfg.offset_type_mode,
            inst_cfg.max_inventory,inst_cfg.max_volume,
            inst_cfg.max_cancel,inst_cfg.max_net_pos,
            inst_cfg.num_inst
        );

        // inst_list -> inst_id_list
        inst_cfg.inst_id_list.clear();
        for (auto& v : inst_cfg.inst_list) {
            inst_cfg.inst_id_list.push_back(instid_map[v]);
            fmt::print("{}(id={}) ", v.data(), instid_map[v]);
        }
        inst_cfg.inst_id_list.shrink_to_fit();
        fmt::print("]\n");

        v_iConfig.push_back(inst_cfg);
    }
    v_iConfig.shrink_to_fit();
}


void Tools::reload_instrument_config(InstrumentConfig* inst_cfg)
{
    Config cfg;
    if (!read_cfg_file(cfg, "config/system.cfg")) {
        return;
    }
    const auto& root = get_cfg_root(cfg);
    const auto& inst_cfgs = get_cfg_member(root, "config.strategy.instrument");

    int i = inst_cfg->inst_id;
    get_cfg_item<double>(inst_cfgs[i], "exchange_rebate_rate", inst_cfg->exchange_rebate_rate);
    get_cfg_item<int>(inst_cfgs[i], "offset_type_mode", inst_cfg->offset_type_mode);
    get_cfg_item<int>(inst_cfgs[i], "max_inventory", inst_cfg->max_inventory);
    get_cfg_item<int>(inst_cfgs[i], "max_volume", inst_cfg->max_volume);
    get_cfg_item<int>(inst_cfgs[i], "max_net_pos", inst_cfg->max_net_pos);
    get_cfg_item<int>(inst_cfgs[i], "max_cancel", inst_cfg->max_cancel);
    
    const auto& gConfig = SYSTEM::get_system_cfg();
    inst_cfg->final_rebate_rate = inst_cfg->exchange_rebate_rate * gConfig.rebate_rate;

    fmt::print("[instrument {}] inst={},final_rebate_rate={:.2},"
        "offset_type_mode={},max_inventory={},max_volume={},"
        "max_cancel={},max_net_pos={}\n",
        inst_cfg->inst_id,inst_cfg->inst.data(),
        inst_cfg->final_rebate_rate,
        inst_cfg->offset_type_mode,
        inst_cfg->max_inventory,
        inst_cfg->max_volume,
        inst_cfg->max_cancel,
        inst_cfg->max_net_pos
    );
}

void Tools::reload_thread_config(int thread_id)
{
    Config cfg;
    if (!read_cfg_file(cfg, "config/system.cfg")) {
        return;
    }
    const auto& root = get_cfg_root(cfg);
    const auto& thread_cfgs = get_cfg_member(root, "config.strategy.thread");
    
    auto& thread_cfg = SYSTEM::get_thread_cfg(thread_id);
    Tools::get_cfg_array<int>(thread_cfgs[thread_id], "allocate_output_msg", thread_cfg.allocate_output);

    fmt::print("[thread {}] bind_cpuid={},strategy_id={},inst:alloc=[ ", 
        thread_id, thread_cfg.cpu_id, thread_cfg.strategy_id
    );

    int m = thread_cfg.inst_list.size();
    for (int j = 0; j < m; ++j) {
        fmt::print("{}:{} ", 
            thread_cfg.inst_list[j].data(), 
            thread_cfg.allocate_output[j]
        );
    }
    fmt::print("]\n");
}

/*
 *  read order.log
 */
void Tools::stringsplit(
    const std::string& str, const char split, 
    std::vector<std::string>& vstr
)
{
    vstr.clear();
    std::string token;
    std::istringstream iss(str);
    while (getline(iss, token, split)) {
        if (!token.empty()) {
            vstr.push_back(token);
        }
    }
}

void Tools::read_order_log()
{
    std::fstream fp;
    char date[9] = { 0 };
    Tools::getdate(date);
    auto path = fmt::format("log/{}/order.log", date);
    fp.open(path.data(), std::ios::in);

    std::map<std::string, int[10]> m_map;
    for (const auto& v : SYSTEM::get_inst_cfgs()) {
        memset(m_map[v.inst], 0, sizeof(m_map[v.inst]));
        m_map[v.inst][0] = v.inst_id;
    }

    std::string line;
    std::vector<std::string> items;
    std::map<std::string, int> idx_map; // column name -> column index
    bool is_first = true;

    while (std::getline(fp, line)) {
        if (line.empty()) { continue; }
        Tools::stringsplit(line, ',', items);
        if (!is_first) 
        {
            auto iter = m_map.find(items[1]);
            if (iter != m_map.end()) {
                iter->second[1] += 1;
                iter->second[2] = std::stoi(items[idx_map["NumInsert"]]);
                iter->second[3] = std::stoi(items[idx_map["NumInsertErr"]]);
                iter->second[4] = std::stoi(items[idx_map["NumCancelGFD"]]);
                iter->second[5] = std::stoi(items[idx_map["NumCancelFAK"]]);
                iter->second[6] = std::stoi(items[idx_map["NumCancelErr"]]);
                iter->second[7] = std::stoi(items[idx_map["NumInfo"]]);
                iter->second[8] += std::stoi(items[idx_map["Filled"]]);
                iter->second[9] = (items[idx_map["Offset"]] == "o") ? 1 : 0;
            }
        }
        else 
        {
            is_first = false;
            for (size_t i = 0; i < items.size(); ++i) {
                const auto& name = items[i];
                if (name == "NumInsert") {
                    idx_map.emplace(name, i);
                } else if (name == "NumCancelGFD") {
                    idx_map.emplace(name, i);
                } else if (name == "NumCancelFAK") {
                    idx_map.emplace(name, i);
                } else if (name == "NumCancelErr") {
                    idx_map.emplace(name, i);
                } else if (name == "NumInsertErr") {
                    idx_map.emplace(name, i);
                } else if (name == "NumInfo") {
                    idx_map.emplace(name, i);
                } else if (name == "Filled") {
                    idx_map.emplace(name, i);
                } else if (name == "Offset") {
                    idx_map.emplace(name, i);
                }
            }
        }
    }
    fp.close();

    for (auto& iter : m_map) {
        const char* inst = iter.first.data();
        const int* num = iter.second;
        if (num[1] == 0) {
            fmt::print("[read_order_log] {} has no updates\n", inst);
            continue;
        }
        auto& p = SYSTEM::get_inst_cfg(num[0]);
        p.num_insert = num[2];
        p.num_insert_err = num[3];
        p.num_cancel_gfd = num[4];
        p.num_cancel_fak = num[5];
        p.num_cancel_err = num[6];
        p.num_info = num[7];
        p.volume = num[8];

        if (p.offset_type_mode == 1 and num[9] == 1) {
            p.offset_type_mode = 2;
        }

        fmt::print(
            "[read_order_log N={}] {}: num_insert={},num_insert_err={},"
            "num_cancel_gfd={},num_cancel_fak={},num_cancel_err={},"
            "num_info={},volume={},offset_mode={}\n", 
            num[1],p.inst.data(),p.num_insert,p.num_insert_err,
            p.num_cancel_gfd,p.num_cancel_fak,p.num_cancel_err,
            p.num_info,p.volume,p.offset_type_mode
        );
    }
}

void Tools::query_ctp()
{
    Ctp2MiniOSpi spi(true, false, QRY_INSTRUMENT_TYPE::FUTURE_ONLY);
    spi.start();
    spi.stop();
}



template void Tools::get_cfg_item<int>(const Setting& cfg, const char* path, int& out);
template void Tools::get_cfg_item<double>(const Setting& cfg, const char* path, double& out);
template void Tools::get_cfg_array<int>(const Setting& cfg, const char* path, std::vector<int>& out);
template void Tools::get_cfg_array<double>(const Setting& cfg, const char* path, std::vector<double>& out);
