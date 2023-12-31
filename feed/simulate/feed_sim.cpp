
#include "../../tool/tool.h"
#include "feed_sim.h"
#include <cassert>

#ifdef NDEBUG
#undef NDEBUG
#endif

static std::vector<MdFeed> m_vmd;
bool Feed_SIM::running = true;

static void set_timestamp(uint32_t* time, uint16_t* ms, const char* _s) {
    int h = std::atoi(_s);
    int m = std::atoi(_s + 3);
    int s = std::atoi(_s + 6);
    *ms = std::atoi(_s + 9);
    *time = h * 10000 + m * 100 + s;
};

static void loadMD(const char* file)
{
    char line[256] = { 0 };
    const char seps[] = ",";
    FILE* fp = fopen(file, "r");
    int i = 0;
    while (fscanf(fp, "%[^\n] ", line) != EOF) 
    {
        //跳过第一行表头
        if (i == 0)
        {
            continue;
        }

        //跳过空行
        if (strlen(line) == 0)
        {
            continue;
        }

        MdFeed feed = { 0 };
        char* token = nullptr;
        token = strtok(line, seps);
        int j = 0;
        while (token != nullptr) 
        {
            switch (j)
            {
            case 0://instrument
            {
                memcpy(feed.instrument, token, INSTRUMENTLENGTH);
            }break;
            case 1://timestamp
            {
                uint32_t time;
                uint16_t ms;
                set_timestamp(&time, &ms, token);
                feed.time = time;
                feed.ms = ms;
            }break;
            case 2://price
            {
                feed.price = atof(token);
            }break;
            case 3://volume
            {
                feed.volume = atoi(token);
            }break;
            case 4://openint
            {
                feed.openint = atoi(token);
            }break;
            case 5://turnover
            {
                feed.turnover = atof(token);
            }break;
            case 6://bid[0]
            {
                feed.bid[0] = atof(token);
            }break;
            case 7://bid[1]
            {
                feed.bid[1] = atof(token);
            }break;
            case 8://bid[2]
            {
                feed.bid[2] = atof(token);
            }break;
            case 9://bid[3]
            {
                feed.bid[3] = atof(token);
            }break;
            case 10://bid[4]
            {
                feed.bid[4] = atof(token);
            }break;
            case 11://ask[0]
            {
                feed.ask[0] = atof(token);
            }break;
            case 12://ask[1]
            {
                feed.ask[1] = atof(token);
            }break;
            case 13://ask[2]
            {
                feed.ask[2] = atof(token);
            }break;
            case 14://ask[3]
            {
                feed.ask[3] = atof(token);
            }break;
            case 15://ask[4]
            {
                feed.ask[4] = atof(token);
            }break;
            case 16://bidvol[0]
            {
                feed.bidvol[0] = atoi(token);
            }break;
            case 17://bidvol[1]
            {
                feed.bidvol[1] = atoi(token);
            }break;
            case 18://bidvol[2]
            {
                feed.bidvol[2] = atoi(token);
            }break;
            case 19://bidvol[3]
            {
                feed.bidvol[3] = atoi(token);
            }break;
            case 20://bidvol[4]
            {
                feed.bidvol[4] = atoi(token);
            }break;
            case 21://askvol[0]
            {
                feed.askvol[0] = atoi(token);
            }break;
            case 22://askvol[1]
            {
                feed.askvol[1] = atoi(token);
            }break;
            case 23://askvol[2]
            {
                feed.askvol[2] = atoi(token);
            }break;
            case 24://askvol[3]
            {
                feed.askvol[3] = atoi(token);
            }break;
            case 25://askvol[4]
            {
                feed.askvol[4] = atoi(token);
            }break;
            case 26://LEVEL
            {
                feed.iL = atoi(token);
            }break;
            case 27://ns
            {
                feed.ns = atoll(token);//64位
            }break;
            default:
                break;
            }

            token = strtok(NULL, seps);
            j++;
        }

        m_vmd.push_back(feed);

        i++;
    }
    fclose(fp);
}

static bool cmp(const MdFeed& a, const MdFeed& b) 
{
    return a.ns < b.ns;
}

static void readMDfile()
{
    const auto& gConfig = SYSTEM::get_system_cfg();
    const auto& Insts = gConfig.inst_list;
    m_vmd.clear();

    std::string dir = "./data/";
    dir += Tools::m_date;
    dir += "/";

    for (auto it = Insts.begin(); it != Insts.end(); it++)
    {
        std::string fname = dir + *it;
        loadMD(fname.c_str());
    }

    std::sort(m_vmd.begin(), m_vmd.end(), cmp);
    m_vmd.shrink_to_fit();
}

void Feed_SIM::start()
{
    readMDfile();

    std::thread t_Feed(Feed_SIM::run);
    t_Feed.detach();
}

void Feed_SIM::close()
{
    *(volatile bool*)&running = false;
}

void Feed_SIM::run()
{
#ifdef __SHFE
    SYSTEM::bind_cpuid(CPUID::FEED_SHFE_CPUID, 0);
    auto* q = QUEUE::get_shfe2user();
#endif

#ifdef __DCE
    SYSTEM::bind_cpuid(CPUID::FEED_DCE_CPUID, 0);
    auto* q = QUEUE::get_dce2user();
#endif

#ifdef __CZCE
    SYSTEM::bind_cpuid(CPUID::FEED_CZCE_CPUID, 0);
    auto* q = QUEUE::get_czce2user();
#endif

    if (q) 
    {
        int N = m_vmd.size();
        int n = N / 20;

        fmt::print("Simulation Length: {}\n", N);

        for (int i = 0; i < N; ++i)
        {
            while (Tools::is_lock());

            auto& feed = m_vmd[i];
            const char* inst = feed.instrument;
            feed.p_cfg = SYSTEM::find_inst_cfg(inst);
            assert(feed.p_cfg != nullptr); // 不应该是nullptr
            FEED::set(&feed);

            q->blockPush([&](MdFeed* data) {
                memcpy(data, &feed, sizeof(MdFeed));
            });
            Tools::lock_sim();

            if (i % n == 0) {
                fmt::print("{}%.", 5 * i / n);
            }
        }
        fmt::print("===> Simulate Done\n");
    }
}
