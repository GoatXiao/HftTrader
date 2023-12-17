#include "v10OSpi.h"

#ifdef __OFFER_V10

V10OSpi::V10OSpi(Queue::qCBTOA* cbtoa)
{
    m_qcbtoa = cbtoa;
    for (uint32_t i = 0; i < ORDERPOOL_NUM; ++i) 
    {
        auto& order = Orders[i]; order.order.localid = i;
        memset(order.OrderAction, 0, SIZE_CXL);
        memset(order.InputOrder, 0, SIZE_NEW);

        auto* hIn = (DstarApiHead*)order.InputOrder;
        hIn->ProtocolCode = CMD_API_Req_OrderInsert;
        hIn->DataLen = insert_len;

        auto* pIn = (DstarApiReqOrderInsertField*)(order.InputOrder + header_len);
        pIn->OrderType = DSTAR_API_ORDERTYPE_LIMIT;
        pIn->Hedge = DSTAR_API_HEDGE_SPECULATE;
        pIn->ClientReqId = 0;
        pIn->Reference = i;
        pIn->SeatIndex = 0;
        pIn->MinQty = 0;

        auto* hCx = (DstarApiHead*)order.OrderAction;
        hCx->ProtocolCode = CMD_API_Req_OrderDelete;
        hCx->DataLen = delete_len;
        auto* pCx = (DstarApiReqOrderDeleteField*)(order.OrderAction + header_len);
        pCx->ClientReqId = 0;
        pCx->Reference = i;
        pCx->SeatIndex = 0;
    }
}

V10OSpi::~V10OSpi() 
{
    if (m_pApi) 
    {
        FreeDstarTradeApi(m_pApi);
    }
}

void V10OSpi::sendData(const uint8_t* data, size_t len) 
{
    if (sender.write(data, len)) {}
    else 
    {
        fmt::print("EfviUdpSender send Error: {}\n", sender.getLastError());
    }
}

bool V10OSpi::start() 
{
    DstarApiReqLoginField m_LoginReq;
    strcpy(m_LoginReq.Password, CONFIG::gConfig.PassWord.c_str());//TODO：配置文件
    strcpy(m_LoginReq.AccountNo, CONFIG::gConfig.UserId.c_str());
    strcpy(m_LoginReq.LicenseNo, CONFIG::gConfig.AuthCode.c_str());
    strcpy(m_LoginReq.AppId, CONFIG::gConfig.AppId.c_str());
    //if (!sender.init("enp2s0f0", "10.168.103.109", LOCAL_PORT, "10.168.103.108", 6666)) {//TODO
    //    logeo("EfviUdpSender init failed {}", sender.getLastError());
    //};
    if (!sender.init(CONFIG::gConfig.TradeInterface.c_str(), CONFIG::gConfig.TradeLocalAddr.c_str(), LOCAL_PORT, CONFIG::gConfig.TradeAddr.c_str(), CONFIG::gConfig.TradePort))
    {
        fmt::print("EfviUdpSender init failed {}\n", sender.getLastError());
    };

    m_pApi = CreateDstarTradeApi();
    m_pApi->RegisterSpi(this);
    //m_pApi->RegisterFrontAddress((char*)"10.168.103.108", 6668);//TODO
    m_pApi->RegisterFrontAddress(const_cast<char*>(CONFIG::gConfig.TradeAddr.c_str()), CONFIG::gConfig.TradePort);
    m_pApi->SetApiLogPath((char*)".");
    m_pApi->SetLoginInfo(&m_LoginReq);
    m_pApi->SetSubscribeStartId(-1);
    m_pApi->SetCpuId(CPUID::CALLBACK_CPUID, -1);//TODO
    fmt::print("V10 CB binds to CPU {}\n", (int)(CPUID::CALLBACK_CPUID));

    char systeminfo[1024] = { 0 };
    unsigned int nVersion = 0;
    int nLen = 1024;
    int ret = m_pApi->GetSystemInfo(systeminfo, &nLen, &nVersion);
    if (ret != 0) 
    {
        fmt::print("GetSystemInfo error: {}\n", ret);
    }
    else 
    {
        DstarApiSubmitInfoField pSubmitInfo = { 0 };
        memcpy(pSubmitInfo.SystemInfo, systeminfo, nLen);
        strcpy(pSubmitInfo.AccountNo, m_LoginReq.AccountNo);
        strcpy(pSubmitInfo.LicenseNo, m_LoginReq.LicenseNo);
        strcpy(pSubmitInfo.ClientAppId, m_LoginReq.AppId);
        pSubmitInfo.AuthType = DSTAR_API_AUTHTYPE_DIRECT;
        pSubmitInfo.AuthKeyVersion = nVersion;
        m_pApi->SetSubmitInfo(&pSubmitInfo);

        ret = m_pApi->Init(DSTAR_API_INIT_QUERY);
        if (ret != 0) 
        {
            fmt::print("Api Init error: {}\n", ret);
        }
    }
    return true;
}

bool V10OSpi::stop()
{
    return true;
}

std::vector<std::string> V10OSpi::getInsts()
{
    return std::vector<std::string>();
}

Order& V10OSpi::get_order(uint32_t i) 
{
    return Orders[i].order;
}

bool V10OSpi::cancel_order(const uint32_t& id)
{
    auto& order = Orders[id];
    OrderDelete& ref = order.OrderAction;
    auto* p = (DstarApiReqOrderDeleteField*)(ref + header_len);
    p->AccountIndex = pAccountIndex;
    p->UdpAuthCode = pUdpAuthCode;
    if (p->OrderId > 0) 
    {
        sendData((uint8_t*)ref, SIZE_CXL);
        order.order.state->num_cancel++;
    }

    return true;
}

bool V10OSpi::send_order(Order& order)
{
    State* state = order.state;
    const auto& instrument = state->instrument;
    OrderInsert& ref = Orders[order.localid].InputOrder;
    auto* p = (DstarApiReqOrderInsertField*)(ref + header_len);
    //strncpy(p->ContractNo, state->instrument, sizeof(DstarApiContractNoType) - 1);
    memcpy(p->ContractNo, instrument, strlen(instrument));
    p->ContractIndex = InstNo.fastFind(instrument);
    p->AccountIndex = pAccountIndex;
    p->UdpAuthCode = pUdpAuthCode;

    switch (order.direction) 
    {
    case 'b':
        p->Direct = DSTAR_API_DIRECT_BUY;
        break;
    case 's':
        p->Direct = DSTAR_API_DIRECT_SELL;
        break;
    }

    switch (order.offset) 
    {
    case 'o':
        p->Offset = DSTAR_API_OFFSET_OPEN;
        break;
    case 't':
        break;
    case 'y':
        break;
    case 'c':
        p->Offset = DSTAR_API_OFFSET_CLOSE;
        break;
    }
    p->OrderPrice = order.price;
    if (order.fak != 0)
    {
        p->ValidType = DSTAR_API_VALID_IOC;
        p->OrderQty = order.volume + order.fak;
    }
    else 
    {
        p->ValidType = DSTAR_API_VALID_GFD;
        p->OrderQty = order.volume;
    }
    order.t1 = Timer::tsc();
    sendData((uint8_t*)ref, SIZE_NEW);
    order.t2 = Timer::tsc();
    //order.t3 = state->recvtime;
    state->num_insert++;

    return true;
}

///报单应答
void V10OSpi::OnRspOrderInsert(const DstarApiRspOrderInsertField* pOrderInsert)
{
    if (pOrderInsert->Reference == -1) { return; }
    if (pOrderInsert->ErrCode != 0) 
    {
        if (pOrderInsert->ErrCode == 20018) 
        {
            //因资金不足的错误报单 强平改合约的净头寸
            m_qcbtoa->blockPush([&](Queue::CBTOA* cbtoa) {
                cbtoa->reference_id = pOrderInsert->Reference;
                cbtoa->price = 0.0;
                cbtoa->volume = 0;
                cbtoa->msg_type = CALLBACK_TYPE::ORDER_ERROR;
                });
        }
        else 
        {
            //其他错误报单
            m_qcbtoa->blockPush([&](Queue::CBTOA* cbtoa) {
                cbtoa->reference_id = pOrderInsert->Reference;
                cbtoa->price = 0.0;
                cbtoa->volume = 0;
                cbtoa->msg_type = CALLBACK_TYPE::ORDER_ERROR;
                });
        }
        fmt::print("OnRspOrderInsert: {}, Error: {}\n",
            pOrderInsert->Reference, pOrderInsert->ErrCode
        );
    }
}

///报单通知
void V10OSpi::OnRtnOrder(const DstarApiOrderField* pOrder)
{
    if (pOrder->Reference == -1) { return; }
    if (pOrder->ErrCode == 0) 
    {
        switch (pOrder->OrderState) {
        case DSTAR_API_STATUS_ACCEPT:
        {
            auto* buf = Orders[pOrder->Reference].OrderAction;
            auto* p = (DstarApiReqOrderDeleteField*)(buf + header_len);
            p->OrderId = pOrder->OrderId;
            //订单被确认，已报入  0
            m_qcbtoa->blockPush([&](Queue::CBTOA* cbtoa) {
                cbtoa->reference_id = pOrder->Reference;
                cbtoa->price = pOrder->OrderPrice;
                cbtoa->volume = pOrder->OrderQty;
                cbtoa->msg_type = CALLBACK_TYPE::ORDER_CONFIRM;
                });
            break;
        }
        case DSTAR_API_STATUS_DELETE:
        {
            int cc = pOrder->OrderQty - pOrder->MatchQty;
            if (cc > 0) 
            { 
                m_qcbtoa->blockPush([&](Queue::CBTOA* cbtoa) {
                    cbtoa->reference_id = pOrder->Reference;
                    cbtoa->price = pOrder->OrderPrice;
                        cbtoa->volume = cc;
                    cbtoa->msg_type = CALLBACK_TYPE::ORDER_CANCEL;
                    });
            }
            break;
        }
        case DSTAR_API_STATUS_LEFTDELETE:
        {
            int cc = pOrder->OrderQty - pOrder->MatchQty;
            if (cc > 0) 
            { 
                m_qcbtoa->blockPush([&](Queue::CBTOA* cbtoa) {
                    cbtoa->reference_id = pOrder->Reference;
                    cbtoa->price = pOrder->OrderPrice;
                    cbtoa->volume = cc;
                    cbtoa->msg_type = CALLBACK_TYPE::ORDER_CANCEL;
                    });
            }
            break;
        }
        }
    }
    else 
    {
        m_qcbtoa->blockPush([&](Queue::CBTOA* cbtoa) {
            cbtoa->reference_id = pOrder->Reference;
            cbtoa->price = pOrder->OrderPrice;
            cbtoa->volume = pOrder->OrderQty;
            cbtoa->msg_type = CALLBACK_TYPE::ORDER_ERROR;
            });
        if (pOrder->ErrCode != 711 and pOrder->ErrCode != 679 and pOrder->ErrCode != 710) 
        {
            fmt::print("OnRtnOrder {}, Error {}, Status {}\n",
                pOrder->ContractNo1, pOrder->ErrCode, pOrder->OrderState
            );
        }
    }
}

///成交通知
void V10OSpi::OnRtnMatch(const DstarApiMatchField* pTrade)
{
    if (pTrade->Reference == -1) { return; }
    if (pTrade->MatchQty > 0) 
    {
        //已完成成交 2
        m_qcbtoa->blockPush([&](Queue::CBTOA* cbtoa) {
            cbtoa->reference_id = pTrade->Reference;
            cbtoa->price = pTrade->MatchPrice;
            cbtoa->volume = pTrade->MatchQty;
            cbtoa->msg_type = CALLBACK_TYPE::ORDER_TRADE;
            });
    }
}

///错误应答
void V10OSpi::OnRspError(DstarApiErrorCodeType nErrorCode)
{
    fmt::print("OnRspError: {}\n", nErrorCode);
}

///登录请求响应,错误码为0说明用户登录成功。
void V10OSpi::OnRspUserLogin(const DstarApiRspLoginField* pLoginRsp) 
{
    if (pLoginRsp->ErrorCode != 0) {
        fmt::print("OnRspUserLogin user: {}, error: {}\n",
            pLoginRsp->AccountNo, pLoginRsp->ErrorCode
        );
    }
    pAccountIndex = pLoginRsp->AccountIndex;
    pUdpAuthCode = pLoginRsp->UdpAuthCode;
}

///合约响应
void V10OSpi::OnRspContract(const DstarApiContractField* pContract) 
{
    const char* inst = pContract->ContractNo;

    //if (strlen(inst) < 8) { 
    auto* p = STATE::get(inst);
    if (p) 
    {
        InstNo.emplace(p->instrument, pContract->ContractIndex);
    }
    else 
    {
        InstNo.emplace(inst, pContract->ContractIndex);
    }

    InstNo.doneModify();
    //}
}

///持仓快照响应
void V10OSpi::OnRspPosition(const DstarApiPositionField* pPosition) 
{
    if (pPosition) 
    {
        const char* inst = pPosition->ContractNo;
        int b = pPosition->PreBuyQty + pPosition->TodayBuyQty;
        int s = pPosition->PreSellQty + pPosition->TodaySellQty;
        auto* state = STATE::get(inst);
        if (state) {
            int inventory[4] = { b, 0, s, 0 };
            state->set_inventory(inventory);
        }
        else if (b + s > 0) {
            fmt::print("[API]{}: ({}, {})\n", inst, b, s);
        }
    }
}

///资金快照响应
void V10OSpi::OnRspFund(const DstarApiFundField* pFund)
{
    fmt::print("\n=== {} ===\nEquity: {:.2f}\nAvailable: {:.2f}\nMargin: {:.2f}\nFee: {:.2f}\n",
        pFund->AccountNo, pFund->Equity, pFund->Avail, pFund->Margin, pFund->Fee
    );
}

///API准备就绪,只有用户收到此就绪通知时才能进行后续的操作
void V10OSpi::OnApiReady(const DstarApiSerialIdType nSerialId)
{
    char sendbuf[1024] = { 0 };
    DstarApiHead* head = (DstarApiHead*)sendbuf;
    head->DataLen = sizeof(DstarApiReqUdpAuthField);
    head->ProtocolCode = CMD_API_Req_UdpAuth;

    auto* req = (DstarApiReqUdpAuthField*)&sendbuf[header_len];
    req->ReqIdMode = DSTAR_API_REQIDMODE_NOCHECK;
    req->AccountIndex = pAccountIndex;
    req->UdpAuthCode = pUdpAuthCode;
    sendData((uint8_t*)sendbuf, header_len + head->DataLen);
};

#endif
