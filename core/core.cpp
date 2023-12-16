#include "core.h"

#include "logger/logger.h"

#include "feed/feed_shfe.h"
#include "feed/feed_dce.h"
#include "feed/feed_czce.h"
#include "feed/feed_sim.h"

#include "strategy/strategy.h"
#include "agent/agent.h"

Core::Core(int pltype) : m_plt(pltype)
{
    m_pModule = nullptr;
}

Core::~Core()
{
    if (m_pModule)
    {
        m_pModule->close();
        delete m_pModule;
        m_pModule = nullptr;
    }
}

void Core::set_strategy_id(int _id)
{
    strategy_id = _id;
}

void Core::LAUNCH_SYSTEM()
{
    init();

    if (m_pModule)
    {
        m_pModule->start();
    }
}

void Core::init()
{
    switch ((PRODUCTIONLINE_TYPE)m_plt)
    {
    case PLT_LOGGER:
    {
        m_pModule = new Logger();
    }break;
    case PLT_FEED_SIMULATE:
    {
        m_pModule = new Feed_SIM();
    }break;
    case PLT_FEED_SHFE:
    {
        m_pModule = new Feed_SHFE();
    }break;
    case PLT_FEED_DCE:
    {
        m_pModule = new Feed_DCE();
    }break;
    case PLT_FEED_CZCE:
    {
        m_pModule = new Feed_CZCE();
    }break;
    case PLT_AGENT:
    {
        m_pModule = new Agent();
    }break;
    case PLT_USER:
    {
        m_pModule = new Strategy(strategy_id);
    }break;
    default:
        break;
    }
}
