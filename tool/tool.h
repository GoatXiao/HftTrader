
#include "../system/system.h"

class Tools
{
public:
    Tools();
    ~Tools();

public:
    static char m_date[9];
public:
    static std::atomic<bool> simulate_lock;
    static void unlock_sim();
    static void lock_sim();
    static bool is_lock();

    static void setdate(char* _date);
    static void getdate_sim(char* _date);
    static void stringsplit(const std::string&, const char, std::vector<std::string>&);

public:
    static void getdate(char* _date);
    static void read_system_config();
    static void read_order_log();
    static void query_ctp();

public:
    static void reload_instrument_config(volatile InstrumentConfig*);
    static void reload_thread_config(int);

public:
    static bool read_cfg_file(Config&, const char*);
    static Setting& get_cfg_root(const Config&);
    static int get_cfg_length(const Setting&);

    static Setting& get_cfg_member(const Setting&, const char*);

    template<typename T, bool str_type=false>
    static void get_cfg_item(const Setting&, const char*, T&);

    template<typename T, bool str_type=false>
    static void get_cfg_array(const Setting&, const char*, std::vector<T>&);

private:
    static void explodeday(const char* date, int& _y, int& _m, int& _d);
    static void getnextday(char* date, const int& _y, const int& _m, const int& _d);
};
