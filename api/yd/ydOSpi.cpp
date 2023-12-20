#include "ydOSpi.h"

#ifdef __OFFER_YD

YDOSpi::YDOSpi()
{
    m_qcbtoa = QUEUE::get_api2agent();

    const auto& gConfig = SYSTEM::get_system_cfg();
    int n = gConfig.inst_list.size();
    v_Instruments.reserve(n);
    for (int i = 0; i < n; ++i)
    {
        memset(&v_Instruments[i], 0, sizeof(yd_inst));
        m_Instruments.emplace(gConfig.inst_list[i], &v_Instruments[i]);
    }
}

YDOSpi::~YDOSpi() 
{
}

bool YDOSpi::start() 
{
    mApi = makeYDApi("yd.ini");
    if (!mApi->start(this)) 
    {
        fmt::print("YD API start error\n");
    }
    else 
    {
        fmt::print("YD API Version {}\n", mApi->getVersion());
    }
    
    const auto& gConfig = SYSTEM::get_system_cfg();
    if (!sender.init(gConfig.TradeInterface.c_str(), gConfig.TradeLocalAddr.c_str(), 
        LOCAL_PORT, gConfig.TradeAddr.c_str(), gConfig.TradePort))
    {
        fmt::print("EfviSender init Error: {}\n", sender.getLastError());
    };
    return true;
}

bool YDOSpi::stop()
{
    if (mApi) {
        mApi->startDestroy();
    }
    return true;
}

bool YDOSpi::send_order(Order& order)
{
    auto& input = ydOrders[order.orderid].InputOrder;
    input.instrument = v_Instruments[order.inst_id].ref;
    switch (order.direction) {
    case 'b':
        input.direct = 0;
        break;
    case 's':
        input.direct = 1;
        break;
    }
    switch ((ORDER_OFFSET)order.offset) {
    case ORDER_OFFSET::O_OPEN:
        input.offset = 0;
        break;
    case ORDER_OFFSET::O_CLOSETODAY:
        input.offset = 3;
        break;
    case ORDER_OFFSET::O_CLOSEYESTERDAY:
        input.offset = 4;
        break;
    case ORDER_OFFSET::O_CLOSE:
        input.offset = 1;
        break;
    }
    input.volume = order.volume;
    input.price = order.price;
    input.type = order.fak;

    if (sendData((uint8_t*)&input, sizeof(input)))
    {
        order.ns_send = TIMER::tsc();
        return true;
    }
    return false;
}

bool YDOSpi::cancel_order(uint32_t id)
{
    const auto& ref = ydOrders[id].OrderAction;
    if (ref.orderid > 0) 
    {
        return sendData((uint8_t*)&ref, sizeof(ref));
    }
    return false;
}

int64_t YDOSpi::get_sysorderid(uint32_t orderid)
{
    return ydOrders[orderid].OrderAction.orderid;
}

inline bool YDOSpi::sendData(const uint8_t* data, size_t len) 
{
    if (sender.write(data, len)) { return true; }
    fmt::print("EfviSender send Error: {}\n", sender.getLastError());
    return false;
}


void YDOSpi::notifyReadyForLogin(bool hasLoginFailed) 
{
    if (hasLoginFailed) 
    {
        fmt::print("YD hasLoginFailed\n");
    }
    else 
    {
        const auto& gConfig = SYSTEM::get_system_cfg();
        mApi->login(gConfig.UserId.c_str(), gConfig.PassWord.c_str(), 
            gConfig.AppId.c_str(), gConfig.AuthCode.c_str()
        );
    }
}

void YDOSpi::notifyLogin(int errorNo, int maxOrderRef, bool isMonitor) 
{
    if (errorNo != 0) 
    {
        fmt::print("YD notifyLogin error {}\n", errorNo);
    }
    else 
    {
        TRADER::set_OrderID(maxOrderRef);
    }
}

void YDOSpi::notifyFinishInit(void) 
{
#ifdef __SHFE
    m_pExchange = mApi->getExchangeByID("SHFE");
#endif
#ifdef __DCE
    m_pExchange = mApi->getExchangeByID("DCE");
#endif

    const auto* m_pAccount = mApi->getMyAccount();
    fmt::print("\n====== [YD] {} ======\nPreBalance {:.2f}\n",
        m_pAccount->AccountID, m_pAccount->PreBalance
    );

    int Count = mApi->getInstrumentCount();
    for (int i = 0; i < Count; ++i) {
        const auto* p = mApi->getInstrument(i);
        std::string inst = p->InstrumentID;
        auto iter = m_Instruments.find(inst);
        if (iter != m_Instruments.end())
        {
            auto* data = iter.value();
            data->ref = p->InstrumentRef;
        }
    }

    // get pre-position
    Count = mApi->getPrePositionCount();
    for (int i = 0; i < Count; ++i) {
        const auto* pp = mApi->getPrePosition(i);
#ifdef __SHFE
        int idx = (pp->PositionDirection == YD_PD_Long) ? 1 : 3;
#else
        int idx = (pp->PositionDirection == YD_PD_Long) ? 0 : 2;
#endif
        const auto* p = pp->m_pInstrument;
        std::string inst = p->InstrumentID;
        auto iter = m_Instruments.find(inst);
        if (iter != m_Instruments.end())
        {
            auto* data = iter.value();
            data->num[idx] = pp->PrePosition;
        }
        else if (pp->PrePosition > 0)
        {
            auto* data = new yd_inst;
            memset(data, 0, sizeof(yd_inst));
            data->num[idx] = pp->PrePosition;
            m_Instruments.emplace(inst, data);
        }
    }
}

void YDOSpi::notifyCaughtUp(void) 
{
    for (const auto& iter : m_Instruments) 
    {
        const auto* p = iter.second;
        fmt::print("[YD] {}: ({}+{},{}+{})\n",
            iter.first,p->num[0],p->num[1],
            p->num[2],p->num[3]
        );
    }
    unsigned char iheader[16], cheader[16];
    if (mApi->getClientPacketHeader(
        mApi->YD_CLIENT_PACKET_INSERT_ORDER, iheader, 16) == 0) 
    {
        fmt::print("[YD] get Insert Packet Header Failed\n");
    }
    if (mApi->getClientPacketHeader(
        mApi->YD_CLIENT_PACKET_CANCEL_ORDER, cheader, 16) == 0) 
    {
        fmt::print("[YD] get Cancel Packet Header Failed\n");
    }

    for (int i = 0; i < ORDERPOOL_NUM; ++i) {
        auto& order = ydOrders[i];
        memset(&order, 0, sizeof(yd_order));
        order.InputOrder.speculate = 1;
        order.InputOrder.localid = i;

        memcpy(order.InputOrder.header, iheader, 16);
        memcpy(order.OrderAction.header, cheader, 16);
        order.OrderAction.exchange = m_pExchange->ExchangeRef;
    }

	m_bIsInitCompleted = true;
    fmt::print("YD notifyCaughtUp\n");
}


void YDOSpi::notifyOrder(
    const YDOrder* pOrder, 
    const YDInstrument* pInstrument, 
    const YDAccount* pAccount) 
{
	if (m_bIsInitCompleted) 
    {
        switch (pOrder->OrderStatus) 
        {
        case YD_OS_Accepted:
            break;
        case YD_OS_AllTraded:
            break;
        case YD_OS_Queuing:
        {
            int orderid = pOrder->OrderRef;
            ydOrders[orderid].OrderAction.orderid = pOrder->OrderSysID;
            m_qcbtoa->blockPush([&](Queue::CBTOA* cbtoa) {
                cbtoa->reference_id = orderid;
                cbtoa->msg_type = CALLBACK_TYPE::ORDER_CONFIRM;
            });
        } break;
        case YD_OS_Canceled:
        {
            int cc = pOrder->OrderVolume - pOrder->TradeVolume;
            m_qcbtoa->blockPush([&](Queue::CBTOA* cbtoa) {
                cbtoa->reference_id = pOrder->OrderRef;
                cbtoa->volume = cc;
                cbtoa->msg_type = CALLBACK_TYPE::ORDER_CANCEL;
            });
        } break;
        case YD_OS_Rejected:
        {
            /*if (pOrder->ErrorNo == 71027)//资金不足？
            {
                //因资金不足的错误报单 强平改合约的净头寸
                m_qcbtoa->blockPush([&](Queue::CBTOA* cbtoa) {
                    cbtoa->reference_id = pOrder->OrderRef;
                    cbtoa->price = pOrder->Price;
                    cbtoa->volume = pOrder->OrderVolume;
                    cbtoa->msg_type = CBTOA_MSG_TYPE::ORDER_ERROR_FORCECLOSE;
                    });
            }
            else
            {
                //其他错误报单
                m_qcbtoa->blockPush([&](Queue::CBTOA* cbtoa) {
                    cbtoa->reference_id = pOrder->OrderRef;
                    cbtoa->price = pOrder->Price;
                    cbtoa->volume = pOrder->OrderVolume;
                    cbtoa->msg_type = CBTOA_MSG_TYPE::ORDER_ERROR;
                    });
            }
            fmt::print("YD Order {} Rejected: {}\n",
                pInstrument->InstrumentID, pOrder->ErrorNo
            );*/
            m_qcbtoa->blockPush([&](Queue::CBTOA* cbtoa) {
                cbtoa->reference_id = pOrder->OrderRef;
                cbtoa->errid = pOrder->ErrorNo;
                cbtoa->msg_type = CALLBACK_TYPE::ORDER_ERROR;
            });
            fmt::print("[YD_OS_Rejected] {}: {}\n",
                pInstrument->InstrumentID, pOrder->ErrorNo
            );
        } break;
        }
    }
}

void YDOSpi::notifyTrade(
    const YDTrade* pTrade, 
    const YDInstrument* pInstrument, 
    const YDAccount* pAccount) 
{
	if (m_bIsInitCompleted) 
    {
        m_qcbtoa->blockPush([&](Queue::CBTOA* cbtoa) {
            cbtoa->reference_id = pTrade->OrderRef;
            cbtoa->price = pTrade->Price;
            cbtoa->volume = pTrade->Volume;
            cbtoa->msg_type = CALLBACK_TYPE::ORDER_TRADE;
        });
    }
    else
    {
        std::string inst = pInstrument->InstrumentID;
        auto iter = m_Instruments.find(inst);
        if (iter != m_Instruments.end()) 
        {
            auto* ptr = iter.value();
            switch (pTrade->Direction) 
            {
            case YD_D_Buy:
                switch (pTrade->OffsetFlag) 
                {
                case YD_OF_Open:
                    ptr->num[0] += pTrade->Volume;
                    break;
                case YD_OF_CloseYesterday:
                    ptr->num[3] -= pTrade->Volume;
                    break;
                default:
                    ptr->num[2] -= pTrade->Volume;
                    break;
                }
                break;
            case YD_D_Sell:
                switch (pTrade->OffsetFlag) 
                {
                case YD_OF_Open:
                    ptr->num[2] += pTrade->Volume;
                    break;
                case YD_OF_CloseYesterday:
                    ptr->num[1] -= pTrade->Volume;
                    break;
                default:
                    ptr->num[0] -= pTrade->Volume;
                    break;
                }
                break;
            }
        }
    }
}

void YDOSpi::notifyFailedCancelOrder(const YDFailedCancelOrder* pFailedOrder, const YDExchange* pExchange, const YDAccount* pAccount) 
{
	if (m_bIsInitCompleted) {
        m_qcbtoa->blockPush([&](Queue::CBTOA* cbtoa) {
            cbtoa->reference_id = pFailedOrder->OrderRef;
            cbtoa->errid = pFailedOrder->ErrorNo;
            cbtoa->msg_type = CALLBACK_TYPE::ORDER_CANCEL_ERR;
        });
    }
}

#endif // YD
