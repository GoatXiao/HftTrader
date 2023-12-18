
#include "feed_shfe.h"
#include "../../system/system.h"

static void set_timestamp(uint32_t* time, uint32_t _s) {
    int s = _s % 60;
    int m = (_s % 3600) / 60;
    int h = (_s / 3600) % 24 + 8;
    *time = h * 10000 + m * 100 + s;
}

bool Feed_SHFE::running = true;

void Feed_SHFE::start()
{
    std::thread t_recv(Feed_SHFE::run);
    t_recv.detach();
}

void Feed_SHFE::close()
{
    *(volatile bool*)&running = false;
}

void Feed_SHFE::run()
{
    SYSTEM::bind_cpuid(CPUID::FEED_SHFE_CPUID, 0);
}
