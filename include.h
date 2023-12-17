#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <queue>

#include "include/libconfig/libconfig.h++"

#define FMT_HEADER_ONLY
#include "include/fmt/core.h"

#define FMTLOG_HEADER_ONLY
#define FMTLOG_NO_CHECK_LEVEL
#define FMTLOG_QUEUE_SIZE (1024 * 8)
#include "include/fmtlog/fmtlog.h"

#include "compile.h"

using namespace libconfig;

#define INSTRUMENTLENGTH 8  //合约代码长度

/*
 *  系统配置信息
 */
struct SystemConfig
{
    std::string BrokerId;
    std::string UserId;
    std::string PassWord;
    std::string AppId;
    std::string AuthCode;

    std::string MdInterface;
    std::string MdLocalAddr;
    std::string MdRemoteAddr;

    std::string TradeInterface;
    std::string TradeLocalAddr;

    std::string TradeAddr;
    std::string QueryAddr;

    std::vector<std::string> inst_list; // 行情订阅列表
    int num_strategy_threads; // 开启策略线程数

    int MdRemotePort;
    int TradePort;
    int QueryPort;

    double fee_per_volume; // 每手+1毛
    double rebate_rate; // broker返佣比例
    double place_rate;
    double hit_rate;
};

/*
 *  策略线程信息
 */
struct ThreadConfig
{
    std::vector<std::string> inst_list; // 线程订阅合约
    std::vector<int> allocate_output; // 订阅合约是否申请输出
    int strategy_id; // 线程策略类型
    int cpu_id; // must begin from 1
};

/*
 *  合约信息
 */
struct InstrumentConfig
{
    int inst_id;//合约ID
    int num_inst;//关联合约数量
    std::string inst;//合约代码
    std::string param_path;//参数路径
    std::vector<int> inst_id_list;//关联合约ID列表
    std::vector<std::string> inst_list;//关联合约代码列表
    double exchange_rebate_rate{ 0 };//交易所返佣

    // Config
    double final_rebate_rate{ 0 };//实际返佣（*broker）
    int offset_type_mode{ 0 };//开平模式（使用系统自动判断开平的功能时有效）
    int max_inventory{ 1000 };//最大持仓（今仓）
    int max_volume{ 10000 };//最大成交量
    int max_cancel{ 480 };//最大gfd撤单
    int max_net_pos{ 1 };//最大净头存
    
    // Static
    int Multiple{ 0 };//合约乘数
    double PriceTick{ 0 };//最小变动价
    double FeeOpen[2] = { 0, 0 };
    double FeeClose[2] = { 0, 0 };
    double FeeCloseToday[2] = { 0, 0 };
    double UpperLimitPrice{ 0 };//涨停板价
    double LowerLimitPrice{ 0 };//跌停板价

    // Statistics
    int inventory[4]{ 0,0,0,0 };//今多，昨多，今空，昨空
    int position{ 0 };//净头存
    int volume{ 0 };//累计成交量
    int num_insert{ 0 };// 累计成功报单笔数
    int num_insert_err{ 0 };// 累计错误报单笔数
    int num_cancel_gfd{ 0 };// 累计成功撤单笔数gfd
    int num_cancel_fak{ 0 };// 累计成功撤单笔数fak
    int num_cancel_err{ 0 };// 累计错误撤单笔数
    int num_info{ 0 };//累计信息笔数
};

/*
 *  行情切片
 */
struct alignas(INSTRUMENTLENGTH) MdFeed
{
    char instrument[INSTRUMENTLENGTH]{ 0 };
    InstrumentConfig* p_cfg{ nullptr };
    double turnover{ 0 };
    double price{ 0 };
    double bid[5]{ 0 };
    double ask[5]{ 0 };
    int bidvol[5]{ 0 };
    int askvol[5]{ 0 };
    int openint{ 0 };
    int volume{ 0 };
    uint32_t time{ 0 }; // hhmmss
    uint16_t ms{ 0 }; // ddd
    uint16_t iL{ 0 };
    int64_t ns{ 0 };
};

/*
 *  订单结构
 */
enum ORDER_OFFSET
{
    O_INVALID = 'z',
    O_CLOSEYESTERDAY = 'y',
    O_CLOSETODAY = 't',
    O_CLOSE = 'c',
    O_OPEN = 'o'
};

enum ORDER_STATUS
{
    O_INIT = 'I',
    O_SEND = 'S',
    O_ERROR = 'E',
    O_QUEUEING = 'Q',
    O_CANCELED = 'C',
    O_COMPLETE = 'T',
    O_CANCELING = 'N'
};

struct alignas(64) Order 
{
    uint32_t orderid{ 0 };
    int inst_id{ -1 };  // 合约ID
    void* pUser{ nullptr }; // 系统保留指针
    double price{ 0 };
    int volume{ 0 };
    int filled{ 0 };    // 已成交量
    uint8_t fak{ 0 };   // 0=gfd, 1=fak
    char direction;     // 'b'=buy, 's'=sell
    char offset;        // See ORDER_OFFSET
    char status;        // See ORDER_STATUS
    uint32_t time{ 0 }; // 创建时间（交易所时戳）
    // 以下为系统使用字段
    int64_t ns_init{ 0 }; 
    int64_t ns_send{ 0 }; // - ns_init = 执行穿透延迟(包含IPC延迟)
    int64_t ns_recv{ 0 }; // - ns_send = API回执延迟
#ifdef __SIMULATE
    int rank{ -1 }; // 模拟时排位
    int force{ 0 }; // 是否强制成交
#endif
};



/*
 *  系统调用函数
 */
namespace SYSTEM
{
    void init_system();
    void bind_cpuid(int cpuid, int priority);

    inline SystemConfig& get_system_cfg();
    inline std::vector<ThreadConfig>& get_thread_cfgs();
    inline std::vector<InstrumentConfig>& get_inst_cfgs();
    inline InstrumentConfig* find_inst_cfg(const char*);
    inline InstrumentConfig& get_inst_cfg(int);
    inline ThreadConfig& get_thread_cfg(int);

    // 通过inst_id获取instrumentID
    inline char* get_inst(int inst_id);

    // libconfig API
    bool read_cfg_file(Config& cfg, const char* file_path);
    Setting& get_cfg_root(const Config& cfg);
    int get_cfg_length(const Setting& cfg);

    Setting& get_cfg_member(const Setting& cfg, const char* path);

    template<typename T, bool str_type=false>
    void get_cfg_item(const Setting& cfg, const char* path, T& item);

    template<typename T, bool str_type=false>
    void get_cfg_array(const Setting& cfg, const char* path, std::vector<T>& vec);

    // Terminal Console
    void reload_instrument_config(const char* inst);
    void reload_thread_config(int thread_id);
}

/*
 *  记时器
 */
namespace TIMER
{
    int64_t tsc2ns(int64_t);//纳秒时间
    int64_t tsc(); //寄存器周期数（建议使用这个打戳，后面需要时再转成纳秒）
    int64_t ns(); //实际等于tsc2ns(tsc())
    void init(); //不要用
}

/*
 *  交易
 */
namespace TRADER
{
    // 不要用
    void set_OrderID(int);

    // 控制台-交易开关
    void on_off(const std::vector<std::string>&, bool);
    // 控制台-打印信息
    void print(const std::vector<std::string>&);

    // 根据orderid获取订单引用
    inline Order& get_order(uint32_t orderid);

    // 根据orderid获取sysorderid
    inline int64_t get_sysorderid(uint32_t orderid);

    // 查询direct方向inst_id的挂单总手数
    template<char direct> int get_outstanding_volume(int inst_id);

    // 查询direct方向inst_id的挂单价price的挂单总手数
    template<char direct> int get_outstanding_volume(int inst_id, double price);

    // 传入lambda函数func，处理direct方向inst_id的挂单
    template<char direct, typename Func> void handle_outstanding_order(int inst_id, Func func);

    // 报单：成功返回orderid，失败返回0
    uint32_t send_order(int inst_id, double price, int volume, uint8_t fak, char direction, char offset, void* __this);

    // 撤单：成功返回true，失败返回false
    bool cancel_order(uint32_t orderid);

    /* 
     * 以下是封装过的接口 
     */

    // 结束交易：撤单+平净头存
    uint32_t close_net_position(int inst_id, void* __this);

    // 撤消direct方向inst_id的挂单价price的挂单（price=0时该方向全部撤销）
    template<char direct> void cancel_order(int inst_id, double price = 0);

    // 默认限价单
    // 增加了自动规避自成交的功能，模板第二个参数开启/关闭
    // 配合配置文件的offset_type_mode字段，自动设置订单offset
    template<char direct, bool self_trade_guard> uint32_t send_order(int inst_id, double price, int volume, void* __this);
}
