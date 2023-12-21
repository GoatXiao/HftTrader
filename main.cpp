#include "core/core.h"
#include "tool/tool.h"

int main(int argc, char* argv[])
{
    char date[DATE_TIME_LEN] = { 0 };

    int nProcs = get_nprocs();
    fmt::print("[Cores]: {}\n", nProcs);

#ifdef __SIMULATE
    if (argc == 2)
    {
        Tools::setdate(argv[1]);
    }
    Tools::getdate_sim(date);
    fmt::print("[SimulateDay]:{}\n", date);
#else
    Tools::getdate(date);
    fmt::print("[TradingDay]:{}\n", date);
#endif // __SIMULATE
    
    SYSTEM::init_system();
    
    //启动日志流水线
    Core* pLogger = new Core(PLT_LOGGER);
    pLogger->LAUNCH_SYSTEM();

#ifdef __SIMULATE
    Core* pSIM = new Core(PLT_FEED_SIMULATE);
    pSIM->LAUNCH_SYSTEM();
#else
    //启动SHFE行情
    Core* pSHFE = new Core(PLT_FEED_SHFE);
    pSHFE->LAUNCH_SYSTEM();

    //启动DCE行情
    Core* pDCE = new Core(PLT_FEED_DCE);
    pDCE->LAUNCH_SYSTEM();

    //启动CZCE行情
    Core* pCZCE = new Core(PLT_FEED_CZCE);
    pCZCE->LAUNCH_SYSTEM();
#endif

    //启动执行线程和回报线程
    Core* pAgent = new Core(PLT_AGENT);
    pAgent->LAUNCH_SYSTEM();

    //启动用户策略线程
    const auto& gConfig = SYSTEM::get_system_cfg();
    int num_threads = gConfig.num_strategy_threads;
    std::vector<Core*> vpUser;
    vpUser.reserve(num_threads);
    for (int i = 0; i < num_threads; ++i) {
        vpUser[i] = new Core(PLT_USER);
        vpUser[i]->set_strategy_id(i);
        vpUser[i]->LAUNCH_SYSTEM();
    }

    //控制台
    SYSTEM::bind_cpuid(CPUID::CONSOLE_CPUID, 0);

    std::vector<std::string> vInsts;
    std::string cmd;

    do {
        std::cin >> cmd;
        if (cmd == "ns") 
        {
            auto ns1 = TIMER::ns(); 
            auto ns2 = TIMER::ns();
            fmt::print("{}\n", ns2 - ns1);
        }
        else if (cmd == "tsc") 
        {
            auto ns1 = TIMER::tsc(); 
            auto ns2 = TIMER::tsc();
            auto ns = TIMER::tsc2ns(ns2) - TIMER::tsc2ns(ns1);
            fmt::print("tsc {}, ns {}\n", ns2 - ns1, ns);
        }
#ifdef __SIMULATE
        else if (cmd == "begin")
        {
            Tools::unlock_sim();
        }
#else
        else if (cmd == "print")
        {
            fmt::print("Enter all or a list of instruments separated by \";\"\n");
            vInsts.clear();
            std::cin >> cmd;
            Tools::stringsplit(cmd, ';', vInsts);
            TRADER::print(vInsts);
        }
        else if (cmd == "off")
        {
            fmt::print("Enter all or a list of instruments separated by \";\"\n");
            vInsts.clear();
            std::cin >> cmd;
            Tools::stringsplit(cmd, ';', vInsts);
            TRADER::on_off(vInsts, false);
        }
        else if (cmd == "on")
        {
            fmt::print("Enter all or a list of instruments separated by \";\"\n");
            vInsts.clear();
            std::cin >> cmd;
            Tools::stringsplit(cmd, ';', vInsts);
            TRADER::on_off(vInsts, true);
        }
        else if (cmd == "reload")
        {
            fmt::print("Enter inst or thread:\n");
            std::cin >> cmd;
            if (cmd == "inst") {
                fmt::print("[reload instrument cfg] Enter instrument:\n");
                std::cin >> cmd;
                SYSTEM::reload_instrument_config(cmd.data());
            }
            else if (cmd == "thread") {
                fmt::print("[reload thread cfg] Enter thread_id:\n");
                std::cin >> cmd;
                SYSTEM::reload_thread_config(std::stoi(cmd));
            }
        }
#endif // __SIMULATE
    } while (cmd != "exit");
    
#ifdef __SIMULATE
    delete pSIM;
#else
    delete pSHFE;
    delete pDCE;
    delete pCZCE;
#endif
    delete pAgent;
    delete pLogger;

    for (int i = 0; i < num_threads; ++i) {
        delete vpUser[i];
    }

    return 0;
}

