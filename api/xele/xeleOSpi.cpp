#include "xeleOSpi.h"

#ifdef __OFFER_XELE

int64_t XeleOSpi::LastSendTimeNs = 0;
uint32_t XeleOSpi::LocalID = 0;

XeleOSpi::XeleOSpi()
{
    m_qcbtoa = QUEUE::get_api2agent();
    for (uint32_t i = 0; i < ORDERPOOL_NUM; i++) 
    {
        auto& order = Orders[i]; order.order.orderid = i;
        memset(&order.InputOrder, 0, sizeof(order.InputOrder));
        memset(&order.OrderAction, 0, sizeof(order.OrderAction));
        order.InputOrder.MessageId = 0x65;
        order.InputOrder.OrderLocalNo = i;
        order.OrderAction.MessageId = 0x67;
        order.OrderAction.ActionFlag = '0';
        order.OrderAction.ActionLocalNo = i;
        order.OrderAction.OrderLocalNo = i;
        order.OrderAction.OrderSysNo = -1;
    }
}

XeleOSpi::~XeleOSpi() 
{
    if (mApi) { mApi->stop(); }
    for (auto& iter : Instruments) 
    {
        delete iter.second;
    }
    Instruments.clear();
}

void XeleOSpi::sendData(const uint8_t* data, size_t len) 
{
    if (sender.write(data, len)) 
    {
        LastSendTimeNs = TIMER::tsc();
    }
    else {
        fmt::print("EfviUdpSender send Error: {}\n", sender.getLastError());
    }
}

bool XeleOSpi::start() 
{
    mApi = makeXTFApi("xele.config");
    if (mApi) 
    {
        fmt::print("xele api version: {}\n", getXTFVersion());
#ifdef __ZH
        setXTFLogEnabled(false);
#endif
        mApi->start(this);
    }
    else 
    {
        fmt::print("xele api start failed\n");
    }
    //const char* addr = mApi->getConfig("TRADE_SERVER_IP");
    //const char* port = mApi->getConfig("TRADE_SERVER_PORT");

    //if (!sender.init("enp101s0f1", "192.168.41.137", LOCAL_PORT, addr, std::atoi(port))) {//TODO：配置文件
    //    logeo("EfviUdpSender init Error: {}", sender.getLastError());
    //}
    const auto& gConfig = SYSTEM::get_system_cfg();
    if (!sender.init(gConfig.TradeInterface.c_str(), gConfig.TradeLocalAddr.c_str(), LOCAL_PORT, gConfig.TradeAddr.c_str(), gConfig.TradePort))
    {
        fmt::print("EfviUdpSender init Error: {}\n", sender.getLastError());
    }
    return true;
}

bool XeleOSpi::stop()
{
    return true;
}

std::vector<std::string> XeleOSpi::getInsts()
{
    return std::vector<std::string>();
}

Order& XeleOSpi::get_order(uint32_t i) 
{
    return Orders[i].order;
}

bool XeleOSpi::send_order(Order& order)
{
    const auto& instrument = SYSTEM::get_inst(order.inst_id);
    auto& input = Orders[order.orderid].InputOrder;
    const auto* inst = Instruments.fastFind(instrument);
    memcpy(input.InstrumentID, instrument, strlen(instrument));
    input.InstrumentIndex = inst->index;
    input.ClientIndex = inst->clientIndex;
    input.Token = inst->clientToken;
    input.RequestID = ++mRequestID;
    input.SeqNo = ++mSeqNo;

    if (order.fak != 0)
    {
        input.VolumeTotalOriginal = order.fak + order.volume;
    }
    else 
    {
        input.VolumeTotalOriginal = order.volume;
    }
    input.LimitPrice = order.price;

    switch (order.direction) 
    {
    case 'b':
        switch (order.offset) 
        {
        case 'o':
            input.InsertType = (order.fak == 0) ? 1 : 9;
            break;
        case 't':
            input.InsertType = (order.fak == 0) ? 5 : 21;
            break;
        case 'y':
            input.InsertType = (order.fak == 0) ? 7 : 27;
            break;
        case 'c':
            input.InsertType = (order.fak == 0) ? 3 : 15;
            break;
        }
        break;
    case 's':
        switch (order.offset) 
        {
        case 'o':
            input.InsertType = (order.fak == 0) ? 2 : 12;
            break;
        case 't':
            input.InsertType = (order.fak == 0) ? 6 : 24;
            break;
        case 'y':
            input.InsertType = (order.fak == 0) ? 8 : 30;
            break;
        case 'c':
            input.InsertType = (order.fak == 0) ? 4 : 18;
            break;
        }
        break;
    }
    sendData((uint8_t*)&input, sizeof(input));
    order.ns_send = TIMER::tsc();

    return true;
}

bool XeleOSpi::cancel_order(const uint32_t& id)
{
    auto& data = Orders[id];
    auto& action = data.OrderAction;
    if (action.OrderSysNo != -1) 
    {
        const auto& instrument = SYSTEM::get_inst(data.order.inst_id);
        const auto* inst = Instruments.fastFind(instrument);
        action.ClientIndex = inst->clientIndex;
        action.Token = inst->clientToken;
        action.RequestID = ++mRequestID;
        action.SeqNo = ++mSeqNo;
        sendData((uint8_t*)&action,
            sizeof(action)
        );
    }

    return true;
}

void XeleOSpi::send_dummy() 
{
    sendData((uint8_t*)dummy, sizeof(dummy));
}

void XeleOSpi::onStart(int errorCode, bool isFirstTime) 
{
    if (errorCode == 0) 
    {
        if (isFirstTime) {}
        //mApi->enableAutoCombinePosition(true);
        const auto& gConfig = SYSTEM::get_system_cfg();
        mApi->login(gConfig.UserId.c_str(), gConfig.PassWord.c_str(), gConfig.AppId.c_str(), gConfig.AuthCode.c_str());
    }
    else 
    {
        fmt::print("API onStart: {}\n", errorCode);
    }
}

void XeleOSpi::onLogin(int errorCode, int exchangeCount) 
{
    fmt::print("API onLogin: {}, #exch {}\n", errorCode, exchangeCount);
}

void XeleOSpi::onError(int errorCode, void* data, size_t size) 
{
    fmt::print("API onError: {}\n", errorCode);
}

void XeleOSpi::onLoadFinished(const XTFAccount* account) 
{
    //set_local_id(account->lastLocalOrderID + 1); //
    LocalID = account->lastLocalOrderID;

    fmt::print("\nAccount: {}\nFee: {:.2f}\nMargin: {:.2f}\nBalance: {:.2f}\n",
        account->accountID, account->commission, account->margin, account->balance
    );

    int n = mApi->getInstrumentCount();
    for (int i = 0; i < n; i++) {
        const auto* instrument = mApi->getInstrument(i);
        const auto* pExch = instrument->getExchange();
        const char* inst = instrument->instrumentID;
        auto lp = instrument->getLongPosition();
        auto sp = instrument->getShortPosition();
        int nby = lp->getYesterdayPosition();
        int nsy = sp->getYesterdayPosition();
        int nb = lp->getAvailablePosition() - nby;
        int ns = sp->getAvailablePosition() - nsy;
        auto* state = STATE::get(inst);
        if (state) {
            int inventory[4] = { nb,nby,ns,nsy };
            state->set_inventory(inventory);
            INSTRUMENT* ptr = new INSTRUMENT;
            memset(ptr, 0, sizeof(INSTRUMENT));
            ptr->index = instrument->instrumentIndex;
            ptr->clientIndex = pExch->clientIndex;
            ptr->clientToken = pExch->clientToken;
            Instruments.emplace(state->instrument, ptr);
        }
        else if (nb + nby + ns + nsy > 0) {
            fmt::print("[API]{}: ({}+{},{}+{})\n",
                inst, nb, nby, ns, nsy
            );
        }
    }
    Instruments.doneModify();

    int rt = mApi->buildWarmOrder(dummy, 64);
    if (rt) {
        fmt::print("buildWarmOrder failed {}\n", rt);
    }
}

void XeleOSpi::onOrder(int errorCode, const XTFOrder* order) 
{
    uint32_t i = order->localOrderID;
    if (
        i == 0 or
        i == 0xd8888888 or
        i == 0x88888888 or
        order->isHistory
        ) {
        return;
    }
    if (errorCode == 0) {
        // 收到报单回报。根据报单状态判断是报单还是撤单。
        switch (order->orderStatus) {
        case XTF_OS_Created:
            break;
        case XTF_OS_Invalid:
            break;
        case XTF_OS_Rejected:
        {
            m_qcbtoa->blockPush([&](Queue::CBTOA* cbtoa) {
                cbtoa->reference_id = i;
                cbtoa->price = 0.0;
                cbtoa->volume = 0;
                cbtoa->msg_type = CALLBACK_TYPE::ORDER_ERROR;
                });
        }
            break;
        case XTF_OS_AllTraded:
            break;
        case XTF_OS_Canceled:
            break;
        default:
        {
            Orders[i].OrderAction.OrderSysNo = order->sysOrderID;
            //订单被确认，已报入  0
            m_qcbtoa->blockPush([&](Queue::CBTOA* cbtoa) {
                cbtoa->reference_id = i;
                cbtoa->price = 0.0;
                cbtoa->volume = 0;
                cbtoa->msg_type = CALLBACK_TYPE::ORDER_CONFIRM;
                });
        }
            break;
        }
    }
    else {
        // 报撤单失败。根据报单错误码判断是柜台拒单，还是交易所拒单。
        switch (order->actionType) {
        case XTF_OA_Cancel:
            break;
        default:
            if (errorCode == 1179 or errorCode == 36) 
            {
                //因资金不足的错误报单 强平改合约的净头寸
                m_qcbtoa->blockPush([&](Queue::CBTOA* cbtoa) {
                    cbtoa->reference_id = i;
                    cbtoa->price = 0.0;
                    cbtoa->volume = 0;
                    cbtoa->msg_type = CALLBACK_TYPE::ORDER_ERROR;
                    });
            }
            else 
            {
                //订单被确认，已报入  0
                m_qcbtoa->blockPush([&](Queue::CBTOA* cbtoa) {
                    cbtoa->reference_id = i;
                    cbtoa->price = 0.0;
                    cbtoa->volume = 0;
                    cbtoa->msg_type = CALLBACK_TYPE::ORDER_CONFIRM;
                    });
            }
            logw("Action {}, Error {}, {}",
                order->actionType, errorCode,
                order->instrument->instrumentID
            );
            break;
        }
    }
}

void XeleOSpi::onCancelOrder(int errorCode, const XTFOrder* cancelOrder) 
{
    // 报撤单失败。根据报单错误码判断是柜台拒单，还是交易所拒单。
    if (errorCode == 0 and !cancelOrder->isHistory) {
        int cc = cancelOrder->orderVolume - cancelOrder->totalTradedVolume;
        if (cc > 0) 
        {
            m_qcbtoa->blockPush([&](Queue::CBTOA* cbtoa) {
                cbtoa->reference_id = cancelOrder->localOrderID;
                cbtoa->price = cancelOrder->orderPrice;
                cbtoa->volume = cc;
                cbtoa->msg_type = CALLBACK_TYPE::ORDER_CANCEL;
                });
        }
    }
}

void XeleOSpi::onTrade(const XTFTrade* trade) 
{
    if (!trade->isHistory) {
        if (trade->tradeVolume > 0) 
        {
            //已完成成交 2
            m_qcbtoa->blockPush([&](Queue::CBTOA* cbtoa) {
                cbtoa->reference_id = trade->order->localOrderID;
                cbtoa->price = trade->tradeVolume;
                cbtoa->volume = trade->tradePrice;
                cbtoa->msg_type = CALLBACK_TYPE::ORDER_TRADE;
                });
        }
        if (trade->isSelfTraded()) 
        {
            const XTFOrder* order = trade->order;
            if (order->orderType == XTF_ODT_FAK) { return; }
            logw("[SELFTRADE] {}, {}, {}, {:.2f}",
                order->localOrderID, order->instrument->instrumentID,
                trade->tradeVolume, trade->tradePrice
            );
        }
    }
}
#endif // XELE