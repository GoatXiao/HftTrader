
#include "feed_czce.h"
#include "../../system/system.h"

inline uint16_t to_ms(uint32_t us) { return us / 1000; }

bool Feed_CZCE::running = true;

void Feed_CZCE::start()
{
    std::thread t_recv(Feed_CZCE::run);
    t_recv.detach();
}

void Feed_CZCE::close()
{
    *(volatile bool*)&running = false;
}

void Feed_CZCE::run()
{
    SYSTEM::bind_cpuid(CPUID::FEED_CZCE_CPUID, 0);
}
