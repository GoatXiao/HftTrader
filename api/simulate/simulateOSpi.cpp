
#include "simulateOSpi.h"

#ifdef __SIMULATE

static int DOUBLE_COMPARE(double a, double b)
{
    const double EPS = 1e-6;
    if (fabs(a - b) <= EPS)
    {
        return 0;
    }
    else if (a > b)
    {
        return 1;
    }
    else
    {
        return -1;
    }
}

SimulateOSpi::SimulateOSpi()
{
    m_qcbtoa = QUEUE::get_api2agent();

    const auto& gConfig = SYSTEM::get_system_config();
    int n = gConfig.inst_list.size();

    m_History.resize(n);
    m_Cancel.resize(n);
    m_Insert.resize(n);

    for (int i = 0; i < n; ++i) {
        memset(&m_History[i], 0, sizeof(MdFeed);
        m_Cancel[i] = new CancelBuffer;
        m_Insert[i] = new InsertBuffer;
        m_Cancel[i]->clear();
        m_Insert[i]->clear();
    }

    hit_rate = gConfig.hit_rate;
    place_rate = gConfig.place_rate;
    fmt::print("[Sim] hit={:.2f}, place={:.2f}\n", hit_rate, place_rate);
}

SimulateOSpi::~SimulateOSpi()
{
}

bool SimulateOSpi::start()
{
    return true;
}

bool SimulateOSpi::stop()
{
    return true;
}

int64_t SimulateOSpi::get_sysorderid(uint32_t orderid)
{
    return -1;
}

bool SimulateOSpi::send_order(Order& order)
{
    auto* buffer = m_Insert[order.inst_id];
    buffer->push_back(&order);
    return true;
}

bool SimulateOSpi::cancel_order(uint32_t id)
{
    auto& order = TRADER::get_order(id);
    auto* buffer = m_Cancel[order.inst_id];
    buffer->push_back(&order);
    return true;
}

void SimulateOSpi::handle_order(int inst_id)
{
    const char* instrument = SYSTEM::get_inst(inst_id);

    auto* p_now = FEED::get(inst_id);

    // wait until L2
    if (p_now->iL == 1) {
        return;
    }

    auto* p_hist = &m_History[inst_id];

    // handle cancel
    auto* cbuffer = m_Cancel[inst_id];
    for (auto* v : *cbuffer) {
        if (m_insertorderbook.erase(v->orderid)) {
            OnOrderCancel(*v);
        }
    }
    cbuffer->clear();

    // handle insert
    auto* ibuffer = m_Insert[inst_id];
    for (auto* p : *ibuffer) {
        m_insertorderbook[p->localid] = p;
        OnOrderInsert(*p);
    }
    ibuffer->clear();

    // 计算成交均价/买/卖
    int market_sell{ 0 };
    int market_buy{ 0 };
    double price{ 0 };

    int trade_size = p_now->volume - p_hist->volume;
    if (trade_size > 0) {
        const auto& cfg = SYSTEM::get_inst_cfg(inst_id);
        price = p_now->turnover - p_hist->turnover;
        price /= cfg.Multiple * trade_size;
        if (DOUBLE_COMPARE(price, 0.0) == -1) {
            fmt::print("[SIM ERROR] price={:.3} < 0: {}, {:.13}, {:.13}\n",
                price, trade_size, p_now->turnover, p_hist->turnover
            );
            return;
        }
        double r = (price - p_hist->bid[0]) / (p_hist->ask[0] - p_hist->bid[0]);
        r = (r > 1.0) ? 1.0 : (r < 0.0) ? 0.0 : r;
        market_buy = std::nearbyint(r * trade_size);
        market_sell = trade_size - market_buy;
    }

    // 了结订单编号
    std::vector<int> done_id;
    done_id.clear();

    // 逐一处理订单
    for (auto it = m_insertorderbook.begin(); it != m_insertorderbook.end(); ++it)
    {
        Order* order = it->second;
        const char* inst = SYSTEM::get_inst(order->inst_id);
        if (strcmp(inst, instrument) != 0) {
            continue;// 非当前合约订单，跳过
        }

        int deal_size = handle_deal(order, p_now, p_hist, price, market_sell, market_buy);
        if (deal_size > 0) {
            OnOrderTrade(*order, order->price, deal_size); //模拟盘中,成交价=挂单价
            if (order->volume - order->filled == deal_size)   // 订单结束
            {
                done_id.push_back(order->localid);
            }
        }
    } // end for loop

    for (const auto& i : done_id) {
        m_insertorderbook.erase(i); // 订单了结
    }

    memcpy(p_hist, p_now, sizeof(MdFeed));
}

void SimulateOSpi::OnOrderInsert(const Order& order)
{
    if (m_qcbtoa)
    {
        m_qcbtoa->blockPush([&](Queue::CBTOA* cbtoa) {
            cbtoa->reference_id = order.localid;
            cbtoa->msg_type = CALLBACK_TYPE::ORDER_CONFIRM;
        });
    }
    
}

void SimulateOSpi::OnOrderCancel(const Order& order)
{
    if (m_qcbtoa)
    {
        m_qcbtoa->blockPush([&](Queue::CBTOA* cbtoa) {
            cbtoa->reference_id = order.localid;
            cbtoa->volume = order.volume - order.filled;
            cbtoa->msg_type = CALLBACK_TYPE::ORDER_CANCEL;
        });
    }
}

void SimulateOSpi::OnOrderTrade(const Order& order, double price, int volume)
{
    if (m_qcbtoa)
    {
        m_qcbtoa->blockPush([&](Queue::CBTOA* cbtoa) {
            cbtoa->reference_id = order.localid;
            cbtoa->price = price;
            cbtoa->volume = volume;
            cbtoa->msg_type = CALLBACK_TYPE::ORDER_TRADE;
        });
    }
}

int SimulateOSpi::handle_deal(
    Order* order, 
    const FEED* now, 
    const FEED* prev,
    double trade_price, 
    int msell, // trades towards bid
    int mbuy // trades towards ask
)
{
    // 措合函数，更新排位，输出成交量
    bool valid_price = DOUBLE_COMPARE(trade_price, 0.0) == 1;
    int size = order->volume - order->filled;
    double price = order->price;
    int& rank = order->rank; // reference
    int deal{ 0 };

    switch (order->direction)
    {
    case 'b': // buy
    {
        int order_type = DOUBLE_COMPARE(price, prev->bid[0]);
        if (order->force == 1 && DOUBLE_COMPARE(price, prev->ask[0]) != -1) // price >= previous ask1 (forced)
        {
            deal = size;
            rank = 0;
        }
        else if (DOUBLE_COMPARE(price, now->ask) != -1) // price >= current ask1
        {
            deal = std::min(msell + now->askvol[0], size);
            rank = 0;
        }
        else if (DOUBLE_COMPARE(price, trade_price) == 1 && valid_price) // current ask1 > price > trade_price
        {
            deal = std::min(msell, size);
            rank = 0;
        }
        else if (DOUBLE_COMPARE(price, now->bid) == 1) // current bid1 < price < current ask1
        {
            if (order_type == -1) // price < previous bid1
            {
                rank = 0; // inside spread
                return 0; // assume no deal
            }
            if (rank == -1) // new order
            {
                switch (order_type)
                {
                case 1: // price > previous bid1
                    rank = std::nearbyint(mbuy * hit_rate); 
                    break;
                case 0: // price == previous bid1
                    rank = prev->bidvol[0];
                    break;
                }
            }
            int n = msell - rank;
            if (n > 0) {
                deal = std::min(n, size);
            }
            rank = 0; // inside spread
        }
        else if (DOUBLE_COMPARE(price, now->bid) == 0) // price == current bid1
        {
            if (order_type == -1) // price < previous bid1
            {
                if (rank == -1) // new order
                {
                    rank = std::nearbyint(now->bidvol[0] * place_rate);
                } 
                else // queueing order
                {
                    int _rank = std::nearbyint(now->bidvol[0] * place_rate);
                    rank = std::min(rank, _rank);
                }
                return 0; // assume no deal
            }
            if (rank == -1) // new order
            {
                switch (order_type) 
                {
                case 1: // price > previous bid1
                    rank = std::nearbyint((mbuy + now->bidvol[0]) * hit_rate);
                    break;
                case 0: // price == previous bid1
                    rank = prev->bidvol[0];
                    break;
                }
            }
            int n;
            if (DOUBLE_COMPARE(price, prev->ask[0]) != -1) // price >= previous ask1
            {
                n = msell + std::min(mbuy, prev->askvol[0]) - rank;
            }
            else // previous ask1 > price >= previous bid1
            {
                n = msell - rank;
            }
            if (n > 0) {
                deal = std::min(n, size); 
                rank = 0;
            } else {
                rank = std::min(now->bidvol[0], -n);
            }
        }
        else // price < current bid1
        {
            for (int i = 1; i < 5; ++i) {
                if (DOUBLE_COMPARE(price, now->bid[i]) == 0) // at which price level
                {
                    if (rank == -1) // new order
                    {
                        switch (order_type) 
                        {
                        case 1: // price > previous bid1
                            rank = std::nearbyint(now->bidvol[i] * hit_rate);
                            break;
                        default: // price <= previous bid1
                            rank = std::nearbyint(now->bidvol[i] * place_rate);
                            break;
                        }
                    } else { // queueing order
                        rank = std::min(now->bidvol[i], rank);
                    }
                }
            }
        }
    } break;
    case 's': // sell
    {
        int order_type = DOUBLE_COMPARE(price, prev->ask[0]);
        if (order->force == 1 && DOUBLE_COMPARE(price, prev->bid[0]) != 1) // price <= previous bid1 (forced)
        {
            deal = size;
            rank = 0;
        }
        else if (DOUBLE_COMPARE(price, now->bid) != 1) // price <= current bid1
        {
            deal = std::min(mbuy + now->bidvol[0], size);
            rank = 0;
        }
        else if (DOUBLE_COMPARE(price, trade_price) == -1 && valid_price) // trade_price > price > current bid1
        {
            deal = std::min(mbuy, size);
            rank = 0;
        }
        else if (DOUBLE_COMPARE(price, now->ask) == -1) // current ask1 > price > current bid1
        {
            if (order_type == 1) // price > previous ask1
            {
                rank = 0; // inside spread
                return 0; // assume no deal
            }
            if (rank == -1) // new order
            {
                switch (order_type)
                {
                case -1: // price < previous ask1
                    rank = std::nearbyint(msell * hit_rate); 
                    break;
                case 0: // price == previous ask1
                    rank = prev->askvol[0];
                    break;
                }
            }
            int n = mbuy - rank;
            if (n > 0) {
                deal = std::min(n, size);
            }
            rank = 0; // inside spread
        }
        else if (DOUBLE_COMPARE(price, now->ask) == 0) // price == current ask1
        {
            if (order_type == 1) // price > previous ask1
            {
                if (rank == -1) // new order
                {
                    rank = std::nearbyint(now->askvol[0] * place_rate);
                } 
                else // queueing order
                {
                    int _rank = std::nearbyint(now->askvol[0] * place_rate);
                    rank = std::min(rank, _rank);
                }
                return 0; // assume no deal
            }
            if (rank == -1) // new order
            {
                switch (order_type) 
                {
                case -1: // price < previous ask1
                    rank = std::nearbyint((msell + now->askvol[0]) * hit_rate);
                    break;
                case 0: // price == previous ask1
                    rank = prev->askvol[0];
                    break;
                }
            }
            int n;
            if (DOUBLE_COMPARE(price, prev->bid[0]) != 1) // price <= previous bid1
            {
                n = mbuy + std::min(msell, prev->bidvol[0]) - rank;
            }
            else // previous bid1 < price <= previous ask1
            {
                n = mbuy - rank;
            }
            if (n > 0) {
                deal = std::min(n, size); 
                rank = 0;
            } else {
                rank = std::min(now->askvol[0], -n);
            }
        }
        else // price > current ask1
        {
            for (int i = 1; i < 5; ++i) {
                if (DOUBLE_COMPARE(price, now->ask[i]) == 0) // at which price level
                {
                    if (rank == -1) // new order
                    {
                        switch (order_type) 
                        {
                        case -1: // price < previous ask1
                            rank = std::nearbyint(now->askvol[i] * hit_rate);
                            break;
                        default: // price >= previous ask1
                            rank = std::nearbyint(now->askvol[i] * place_rate);
                            break;
                        }
                    } 
                    else // queueing order
                    {
                        rank = std::min(now->askvol[i], rank);
                    }
                }
            }
        }
    } break;
    } // end switch(direction)

    return deal;
}

#endif
