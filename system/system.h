#ifndef  __SYSTEM_H__
#define  __SYSTEM_H__

#include "../include.h"
#include "../define.h"

using MAP = tsl::robin_map<
    uint32_t, int, std::hash<uint32_t>, std::equal_to<uint32_t>, 
    std::allocator<std::pair<uint32_t, int>>, true>;

struct alignas(64) OrderMap
{
    MAP SP2V; // ������: price -> size
    MAP LP2V; // ����: price -> size
    
    MAP SI2P; // ������: id -> price
    MAP LI2P; // ����: id -> price

    // �����Ŷ��У�����;���Ķ���: [����]
    int inqueue[2] = {0, 0}; 

    void reset(int n = 256) {
	    SI2P.clear();
	    LI2P.clear();
        SP2V.clear();
	    LP2V.clear();
	    SI2P.reserve(n);
	    LI2P.reserve(n);
        SP2V.reserve(n);
	    LP2V.reserve(n);
        inqueue[0] = 0;
        inqueue[1] = 0;
    }:

    void print() const {
        fmt::print("#OrderByID({},{}),#OrderByPrice({},{}),Total({},{})\n"
            LI2P.size(),SI2P.size(),LP2V.size(),SP2V.size(),inqueue[0],inqueue[1]
        );
    };
    
    template<char d> MAP& get_outstanding_order() {
        if (d == 's') { return SI2P; }
        if (d == 'b') { return LI2P; }
    };

    template<char d> int& get_outstanding_volume() {
        if (d == 's') { return inqueue[1]; }
        if (d == 'b') { return inqueue[0]; }
    };

    template<char d> MAP& get_price_map() {
	    if (d == 's') { return SP2V; }
	    if (d == 'b') { return LP2V; }
    };

    template<char d> int get_outstanding_volume(uint32_t pr) {
        if (0 == get_outstanding_volume<d>()) { return 0; }
        const auto& map = get_price_map<d>();
        const auto& iter = map.find(pr);
        if (iter != map.end()) {
            return iter->second;
        }
        return 0;
    };

    template<char d> void erase_orderid(uint32_t i) {
        if (d == 's') { SI2P.erase(i); }
        if (d == 'b') { LI2P.erase(i); }
    };

    template<char d> void erase_price(uint32_t pr, int v) {
        auto& map = get_price_map<d>();
        auto iter = map.find(pr);
        if (iter != map.end()) {
            if (iter->second > v) {
                iter.value() -= v;
            } else {
                map.erase(pr);
            }
            int& n = get_outstanding_volume<d>();
            if (n >= v) {
                n -= v;
            } else {
                n = 0;
            }
        }
    };

    template<char d> void insert_orderid(uint32_t i, int pr) {
        if (d == 's') { SI2P.insert(std::make_pair(i, pr)); }
        if (d == 'b') { LI2P.insert(std::make_pair(i, pr)); }
    };
    
    template<char d> void insert_price(uint32_t pr, int v) {
        auto& map = get_price_map<d>();
        auto iter = map.find(pr);
        if (iter == map.end()) {
            map.insert(std::make_pair(pr, v));
        } else {
            iter.value() += v;
        }
        get_outstanding_volume<d>() += v;
    };
};

struct alignas(8) InstrumentState 
{
    InstrumentConfig* const p_cfg;
    OrderMap* p_OrderMap;
    double inv_ptick{ 1 };
    double value{ 0 };          // �ۼƷѺ�ӯ����Ԫ��
    double fee{ 0 };            // �ۼ������ѣ�Ԫ��

    double bid{ 0 };
    double ask{ 0 };
    int bidvol{ 0 };
    int askvol{ 0 };
    
    uint32_t timestamp{ 0 };    // ����ʱ�䣨��ʽ��hhmmsszzz��zΪ���룩
    bool balanced{ true };      // ��λ��ʼ����ʶ
    bool trading{ false };      // ���ױ�ʶ
    bool on_off{ false };       // ���׿���
    bool fast1{ false };

    int64_t ns_data{ 0 };       // feed recv
    int64_t ns_signal{ 0 };     // signal sent

    InstrumentState(InstrumentConfig* p) : p_cfg(p) {
        p_OrderMap = new OrderMap;
        reset();
        init();
    };

    void reset() {
        p_OrderMap->reset();
        trading = false;
    };

    std::pair<double, double> get_value_fee() const {
        double v{ 0.0 }, f{ 0.0 };
        int mult = p_cfg->Multiple;
        int pos = p_cfg->position;
        if (pos > 0) {
            f = bid * p_cfg->FeeOpen[0] + p_cfg->FeeOpen[1];
            v = (bid - f) * pos;
            f *= pos;
        }
        else if (pos < 0) {
            f = ask * p_cfg->FeeOpen[0] + p_cfg->FeeOpen[1];
            v = (ask + f) * pos;
            f *= -pos;
        }
        f = (fee + f) * mult;
        v = (value + v) * mult;
        return std::make_pair(v, f);
    };

    inline double fbValue(const std::pair<double, double>& p) const {
        return p.first + (p.second - p_cfg->volume * 0.1) * p_cfg->final_rebate_rate;
    };

    void print() const {
        auto p = get_value_fee();
        fmt::print(
            "{}({}),V({:.2f}):{:.2f},v:{:.2f},f:{:.2f},"
            "qvcn({},{},{},{}),t{},pos({}+{},{}+{})\n",
            p_cfg->inst.data(),trading,p_cfg->final_rebate_rate,fbValue(p),
            p.first,p.second,p_cfg->position,p_cfg->volume,p_cfg->num_cancel_gfd,
            p_cfg->num_info,p_cfg->offset_type_mode,p_cfg->inventory[0],
            p_cfg->inventory[1],p_cfg->inventory[2],p_cfg->inventory[3]
        );
        p_OrderMap->print();
    };

    void init() {
        const auto& gSysConfig = SYSTEM::get_system_cfg();

        inv_ptick = 1.0 / p_cfg->PriceTick;

        // ��ƽ����
        p_cfg->FeeOpen[1] = (p_cfg->FeeOpen[1] + 
            gSysConfig.fee_per_volume) / p_cfg->Multiple;

        // ƽ�����
        p_cfg->FeeClose[1] = (p_cfg->FeeClose[1] + 
            gSysConfig.fee_per_volume) / p_cfg->Multiple;

        // ƽ�����
        p_cfg->FeeCloseToday[1] = (p_cfg->FeeCloseToday[1] + 
            gSysConfig.fee_per_volume) / p_cfg->Multiple;

        fmt::print(
            "[{}]({},{:.2f}):fee({:.2e},{:.3f},{:.2e},{:.3f},{:.2e},{:.3f}),"
            "rebate={},max(v{},p{},q{},c{}),offset={}\n",
            p_cfg->inst.data(),p_cfg->Multiple,p_cfg->PriceTick,
            p_cfg->FeeOpen[0],p_cfg->FeeOpen[1],p_cfg->FeeClose[0],p_cfg->FeeClose[1],
            p_cfg->FeeCloseToday[0],p_cfg->FeeCloseToday[1],p_cfg->final_rebate_rate,
            p_cfg->max_volume,p_cfg->max_inventory,p_cfg->max_net_pos,
            p_cfg->max_cancel,p_cfg->offset_type_mode
        );

        set_inventory();
    };

    void set_inventory() {
        p_cfg->position = p_cfg->inventory[0] + p_cfg->inventory[1] - 
            p_cfg->inventory[2] - p_cfg->inventory[3];
        balanced = (p_cfg->position == 0); // ���ֲ� == 0 ?
        fmt::print("[{}](balanced={}):q({}),({}+{},{}+{})\n", 
            p_cfg->inst.data(),balanced,p_cfg->position,
            p_cfg->inventory[0],p_cfg->inventory[1],
            p_cfg->inventory[2],p_cfg->inventory[3]
        );
    };

    void set(uint32_t time, double _b, double _a, int bv, int av) {
        trading = balanced and on_off;
        // ����ʱ����̿�
        timestamp = time;
        bidvol = bv; 
        askvol = av;
        bid = _b; 
        ask = _a;
    };

    void guard() {
        if (trading) {
            if (p_cfg->num_info > 3950) { // ��Ϣ������
                fmt::print("[{}] num_info: {}\n", 
                    p_cfg->inst.data(),
                    p_cfg->num_info
                );
                trading = false;
                on_off = false;
            }
            else if (p_cfg->num_cancel_gfd > p_cfg->max_cancel) { // ��������
                fmt::print("[{}] num_cancel_gfd: {} / {}\n", 
                    p_cfg->inst.data(),
                    p_cfg->num_cancel_gfd,
                    p_cfg->max_cancel
                );
                trading = false;
                on_off = false;
            }
            else if (p_cfg->volume > p_cfg->max_volume) { // �ɽ�������
                fmt::print("[{}] volume: {} / {}\n", 
                    p_cfg->inst.data(),
                    p_cfg->volume,
                    p_cfg->max_volume
                );
                trading = false;
                on_off = false;
            }
            if (p_cfg->offset_type_mode == 2) {
                if (p_cfg->inventory[0] + p_cfg->inventory[2] > p_cfg->max_inventory) {
                    fmt::print("[{}] inventory: ({}+{},{}+{}) / {}\n",
                        p_cfg->inst.data(),p_cfg->inventory[0],
                        p_cfg->inventory[1],p_cfg->inventory[2],
                        p_cfg->inventory[3],p_cfg->max_inventory
                    );
                    trading = false;
                    on_off = false;
                }
            } 
            else if (p_cfg->offset_type_mode == 3) { // ���
                if (p_cfg->inventory[0] + p_cfg->inventory[1] + 
                    p_cfg->inventory[2] + p_cfg->inventory[3] == 0
                ) {
                    fmt::print("[{}] inventory cleared\n", 
                        p_cfg->inst.data()
                    );
                    trading = false;
                    on_off = false;
                }
            }
        }
    };

    /*
     *  ������
     */
    template<char d> inline MAP& get_outstanding_order() {
        return p_OrderMap->get_outstanding_order<d>();
    };

    template<char d> inline int& get_outstanding_volume() {
        return p_OrderMap->get_outstanding_volume<d>();
    };

    template<char d> inline MAP& get_price_map() {
        return p_OrderMap->get_price_map<d>();
    };

    template<char d> inline int get_outstanding_volume(double price) {
        return p_OrderMap->get_outstanding_volume<d>(
            (uint32_t)std::nearbyint(price * inv_ptick)
        );
    };

    template<char d> inline void erase_orderid(uint32_t i) {
        p_OrderMap->erase_orderid<d>(i);
    };

    template<char d> inline void erase_price(double price, int v) {
        p_OrderMap->erase_price<d>((uint32_t)std::nearbyint(price * inv_ptick), v);
    };

    template<char d> inline void insert_orderid(uint32_t i, double price) {
        p_OrderMap->insert_orderid<d>(i, (int)std::nearbyint(price * inv_ptick));
    };
    
    template<char d> inline void insert_price(double price, int v) {
        p_OrderMap->insert_price<d>((uint32_t)std::nearbyint(price * inv_ptick), v);
    };

    /*
     * �ɽ��ر�ʱ���ã�����ʵʱ����
     */
    template<char direct> void update(char offset, double pr, int v, double& _fee) {
        if (direct == 'b') {
            p_cfg->position += v;
            switch ((ORDER_OFFSET)offset) {
            case ORDER_OFFSET::O_OPEN: 
                _fee = pr * p_cfg->FeeOpen[0] + p_cfg->FeeOpen[1];
                p_cfg->inventory[0] += v; 
                break;
            case ORDER_OFFSET::O_CLOSETODAY: 
                _fee = pr * p_cfg->FeeCloseToday[0] + p_cfg->FeeCloseToday[1];
                p_cfg->inventory[2] -= v; 
                break;
            case ORDER_OFFSET::O_CLOSEYESTERDAY: 
                _fee = pr * p_cfg->FeeClose[0] + p_cfg->FeeClose[1];
                p_cfg->inventory[3] -= v; 
                break;
            case ORDER_OFFSET::O_CLOSE: 
                if (p_cfg->offset_type_mode == 0) {
                    _fee = pr * p_cfg->FeeCloseToday[0] + p_cfg->FeeCloseToday[1];
                } else {
                    _fee = pr * p_cfg->FeeClose[0] + p_cfg->FeeClose[1];
                }
                p_cfg->inventory[2] -= v;
                break;
            }
            if (balanced) {
                //pr += _fee;
                value -= (pr + _fee) * v;
            }
        }
        if (direct == 's') {
            p_cfg->position -= v;
            switch ((ORDER_OFFSET)offset) {
            case ORDER_OFFSET::O_OPEN: 
                _fee = pr * p_cfg->FeeOpen[0] + p_cfg->FeeOpen[1];
                p_cfg->inventory[2] += v; 
                break;
            case ORDER_OFFSET::O_CLOSETODAY: 
                _fee = pr * p_cfg->FeeCloseToday[0] + p_cfg->FeeCloseToday[1];
                p_cfg->inventory[0] -= v; 
                break;
            case ORDER_OFFSET::O_CLOSEYESTERDAY: 
                _fee = pr * p_cfg->FeeClose[0] + p_cfg->FeeClose[1];
                p_cfg->inventory[1] -= v; 
                break;
            case ORDER_OFFSET::O_CLOSE: 
                if (p_cfg->offset_type_mode == 0) {
                    _fee = pr * p_cfg->FeeCloseToday[0] + p_cfg->FeeCloseToday[1];
                } else {
                    _fee = pr * p_cfg->FeeClose[0] + p_cfg->FeeClose[1];
                }
                p_cfg->inventory[0] -= v; 
                break;
            }
            if (balanced) {
                //pr -= _fee;
                value += (pr - _fee) * v;
            }
        }
        fee += _fee * v;
        p_cfg->volume += v;
        if (!balanced and p_cfg->position == 0) {
	        balanced = true;
        }
    };

    /*
     * ���ݲ�λ�жϿ�ƽ��һ������²���ͨ�ã�
     */
    template<char d> char get_offset(int v) {
        int nt = (d == 'b') ? p_cfg->inventory[2] : p_cfg->inventory[0];
#ifdef __SHFE
        int ny = (d == 'b') ? p_cfg->inventory[3] : p_cfg->inventory[1];
#endif
        switch (p_cfg->offset_type_mode) {
        case 2:
            return ORDER_OFFSET::O_OPEN;
            break;
        case 1:
#ifdef __SHFE
            if (v <= ny) {
                return ORDER_OFFSET::O_CLOSEYESTERDAY;
#else
            if (v <= nt) {
                return ORDER_OFFSET::O_CLOSE;
#endif
            } else {
                p_cfg->offset_type_mode = 2;
                return ORDER_OFFSET::O_OPEN;
            }
            break;
        case 0:
#ifdef __SHFE
            if (v <= ny) {
                return ORDER_OFFSET::O_CLOSEYESTERDAY;
            } else if (v <= nt) {
                return ORDER_OFFSET::O_CLOSETODAY;
#else
            if (v <= nt) {
                return ORDER_OFFSET::O_CLOSE;
#endif
            } else {
                return ORDER_OFFSET::O_OPEN;
            }
            break;
        case 3:
#ifdef __SHFE
            if (v <= ny) {
                return ORDER_OFFSET::O_CLOSEYESTERDAY;
            } else if (v <= nt) {
                return ORDER_OFFSET::O_CLOSETODAY;
#else
            if (v <= nt) {
                return ORDER_OFFSET::O_CLOSE;
#endif
            } else {
                on_off = false;
                trading = false;
                return ORDER_OFFSET::O_INVALID;
            }
            break;
        }
    };
};


/*
 *  ϵͳ����
 */
struct Queue {
    typedef struct alignas(8) {
        uint32_t reference_id{ 0 }; // �������
        int msg_type{ -1 };     // ��Ϣ����
        double price{ 0 };      // �ɽ��� ontradertn
        int volume{ 0 };        // �ɽ���or������ ontrade/oncancel
        int errid{ 0 };         // ������ onsenderr/oncancelerr
    } CBTOA;

    typedef struct alignas(8) {
        const Order* order;
        int num_insert;
        int num_insert_err;
        int num_cancel_gfd;
        int num_cancel_fak;
        int num_cancel_err;
        int num_info;
        int64_t ns;
    } ORDER_LOG_TYPE;

    typedef struct alignas(8) {
        int64_t ns;
        int inst_id;
        uint32_t localid;
        uint32_t insert_time;
        int volume;
        double price;
        double fee;
        char direction;
        char offset;
        char padding[6];
    } TRADE_LOG_TYPE;

    typedef struct alignas(8) {
        int64_t ns;
        int inst_id;
        uint32_t localid;
        double price;
        int volume;
        char direction;
        char offset;
        uint8_t fak;
        char padding[1];
    } SEND_LOG_TYPE;

    typedef struct alignas(8) {
        int64_t ns;
        int inst_id;
        uint32_t localid;
        uint32_t insert_time;
        char padding[4];
    } CANCEL_LOG_TYPE;

    typedef struct alignas(8) {
        int64_t ns;
        int inst_id;
        uint32_t time;
        uint32_t localid;
        int errid = 0;
    } ERR_LOG_TYPE;

    using qATOL = SPSCVarQueueOPT<1024*8>;
    using qCBTOA = SPSCQueueOPT<CBTOA, 1024>;
    using qUTOA = SPSCVarQueueOPT<4096>;

    using qFTOL = SPSCQueueOPT<MdFeed, 1024>;
    using qFTOU = Dsipatcher<MdFeed, 1024>;
};


namespace QUEUE
{
    void init();
    inline Queue::qFTOL* get_shfe2log();
    inline Queue::qFTOL* get_dce2log();
    inline Queue::qFTOL* get_czce2log();
    
    inline Queue::qFTOU* get_shfe2user();
    inline Queue::qFTOU* get_dce2user();
    inline Queue::qFTOU* get_czce2user();

    inline std::vector<Queue::qUTOA*> get_user2agent();
    inline Queue::qUTOA* get_user2agent(int);
    inline Queue::qCBTOA* get_api2agent();
    inline Queue::qATOL* get_agent2log();
}

/*
 *  ����
 */
namespace FEED
{
    inline uint32_t get_timestamp(const MdFeed*);
    void set(const MdFeed*);
    MdFeed* get(int);
}
#endif
