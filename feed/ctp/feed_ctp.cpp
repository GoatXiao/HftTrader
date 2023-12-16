
#include "feed_ctp.h"
#include "../../system/system.h"

static void set_timestamp(uint32_t* time, const char* _s) {
    int h = std::atoi(_s);
    int m = std::atoi(_s + 3);
    int s = std::atoi(_s + 6);
    *time = h * 10000 + m * 100 + s;
}

bool Feed_CTP::running = true;

void Feed_CTP::start()
{
    std::thread t_recv(Feed_CTP::run);
    t_recv.detach();
}

void Feed_CTP::close()
{
    *(volatile bool*)&running = false;
}

void Feed_CTP::run()
{
    SYSTEM::bind_cpuid(CPUID::FEED_CTP_CPUID, 0);
}
