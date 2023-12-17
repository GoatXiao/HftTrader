#include "ctp2miniOSpi.h"

Ctp2MiniOSpi::Ctp2MiniOSpi(bool isqryinit, bool issuballinsts, int qrytype)
{
    m_bIsInitCompleted = false;
    m_bIsUnInitCompleted = false;

    m_bIsQryInit = isqryinit;
    m_bIsSubAllInsts = issuballinsts;
    m_qryType = qrytype;
    
    m_qcbtoa = QUEUE::get_api2agent();

    pUserApi = NULL;
    m_nRequestID = 0;
    m_nFrontID = 0;
    m_nSessionID = 0;
    
    Insts.clear();

    if (!m_bIsQryInit)
    {
        const auto& gConfig = SYSTEM::get_system_config();
        for (int i = 0; i < ORDERPOOL_NUM; i++)
        {
            //报单初始化
            auto& insertReq = ctp2mini_OrderPool[i].InsertReq;
            memset(&insertReq, 0, sizeof(CThostFtdcInputOrderField));
            strcpy(insertReq.BrokerID, gConfig.BrokerId.c_str());
            strcpy(insertReq.InvestorID, gConfig.UserId.c_str());
            sprintf(insertReq.OrderRef, "%012d", i);
            insertReq.OrderPriceType = THOST_FTDC_OPT_LimitPrice;
            insertReq.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;
            insertReq.VolumeCondition = THOST_FTDC_VC_AV;
            insertReq.ContingentCondition = THOST_FTDC_CC_Immediately;
            insertReq.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;
            insertReq.TimeCondition = THOST_FTDC_TC_GFD;

            //撤单初始化
            auto& cancelReq = ctp2mini_OrderPool[i].CancelReq;
            memset(&cancelReq, 0, sizeof(CThostFtdcInputOrderActionField));
            sprintf(cancelReq.OrderRef, "%012d", i);
            cancelReq.FrontID = m_nFrontID;
            cancelReq.SessionID = m_nSessionID;
            cancelReq.ActionFlag = THOST_FTDC_AF_Delete;
        }
    }
}

Ctp2MiniOSpi::~Ctp2MiniOSpi()
{
    pUserApi->Release();
}

bool Ctp2MiniOSpi::start()
{
    if (pUserApi != nullptr) {
        pUserApi->Release();
    }
    pUserApi = CThostFtdcTraderApi::CreateFtdcTraderApi();// 创建UserApi
    pUserApi->RegisterSpi((CThostFtdcTraderSpi*)this);				// 注册事件类
    pUserApi->SubscribePublicTopic(THOST_TERT_QUICK);		// 注册公有流
    pUserApi->SubscribePrivateTopic(THOST_TERT_QUICK);	// 注册私有流

    const auto& gConfig = SYSTEM::get_system_config();
    if (m_bIsQryInit)
    {
        pUserApi->RegisterFront(const_cast<char*>(gConfig.QueryAddr.c_str()));
    }
    else
    {
        pUserApi->RegisterFront(const_cast<char*>(gConfig.TradeAddr.c_str()));
    }
    
    pUserApi->Init();

    while (!m_bIsInitCompleted)
    {
        ;
    }

    return true;
}

bool Ctp2MiniOSpi::stop()
{
    if (pUserApi)
    {
        CThostFtdcUserLogoutField req = { 0 };
        memset(&req, 0, sizeof(CThostFtdcUserLogoutField));
        int iResult = pUserApi->ReqUserLogout(&req, ++m_nRequestID);
        if (iResult != 0)
        {
            fmt::print("\n====== CTP2MINI Trade ReqUserLogout fail ======\n");
            return false;
        }

        while (!m_bIsUnInitCompleted)
        {
            ;
        }

    }
    return true;
}

int64_t Ctp2MiniOSpi::get_sysorderid(uint32_t orderid) 
{
    return ctp2mini_OrderPool[orderid].sysorderid; 
}

bool Ctp2MiniOSpi::send_order(Order& order)
{
    if (pUserApi)
    {
        const char* inst = SYSTEM::get_inst(order.inst_id);
        auto& insertReq = ctp2mini_OrderPool[order.orderid].InsertReq;
        std::memcpy(insertReq.InstrumentID, inst.data(), INSTRUMENTLENGTH);
        
        switch (order.direction)
        {
        case 'b':
            insertReq.Direction = THOST_FTDC_D_Buy;
            break;
        case 's':
            insertReq.Direction = THOST_FTDC_D_Sell;
            break;
        }
        
        switch (order.offset)
        {
        case 'o':
            insertReq.CombOffsetFlag[0] = THOST_FTDC_OF_Open;
            break;
        case 'c':
            insertReq.CombOffsetFlag[0] = THOST_FTDC_OF_Close;
            break;
        case 'y':
            insertReq.CombOffsetFlag[0] = THOST_FTDC_OF_CloseYesterday;
            break;
        case 't':
            insertReq.CombOffsetFlag[0] = THOST_FTDC_OF_CloseToday;
            break;
        }
        insertReq.VolumeTotalOriginal = order.volume;
        insertReq.LimitPrice = order.price;
        
        if (order.fak) 
        {
            ///立即完成，否则撤销
            insertReq.TimeCondition = THOST_FTDC_TC_IOC;
        }
       
        int iResult = pUserApi->ReqOrderInsert(&insertReq, ++m_nRequestID);
        order.ns_send = Timer::tsc();

        if (iResult == 0)
        { 
            return true;
        }
        else
        {
            fmt::print("\n====== CTP2MINI Trade ReqOrderInsert fail ======\n");
            return false;
        }
    }
    return false;
}

bool Ctp2MiniOSpi::cancel_order(uint32_t id)
{
    if (pUserApi)
    {
        int iResult = pUserApi->ReqOrderAction(
            &(ctp2mini_OrderPool[id].CancelReq), 
            ++m_nRequestID
        );
        if (iResult == 0)
        {
            return true;
        }
        else
        {
            fmt::print("\n====== CTP2MINI Trade ReqOrderAction fail ======\n");
            return false;
        }
    }
    return false;
}

bool Ctp2MiniOSpi::IsErrorRspInfo(CThostFtdcRspInfoField* pRspInfo)
{
    // 如果ErrorID != 0, 说明收到了错误的响应
    return ((pRspInfo) && (pRspInfo->ErrorID != 0));
}

void Ctp2MiniOSpi::OnFrontConnected()
{
    const auto& gConfig = SYSTEM::get_system_config();
    fmt::print("\n====== CTP2MINI Trade Connect success ======\n");
    CThostFtdcReqAuthenticateField req = { 0 };
    memset(&req, 0, sizeof(CThostFtdcReqAuthenticateField));
    strcpy(req.BrokerID, gConfig.BrokerId.c_str());
    strcpy(req.AppID, gConfig.AppId.c_str());
    strcpy(req.AuthCode, gConfig.AuthCode.c_str());

    int iResult = pUserApi->ReqAuthenticate(&req, ++m_nRequestID);
    if (iResult != 0)
    {
        fmt::print("\n====== CTP2MINI Trade ReqAuthenticate fail ======\n");
    }
}

void Ctp2MiniOSpi::OnFrontDisconnected(int nReason)
{
    fmt::print("\n====== CTP2MINI Trade OnFrontDisconnected ======\n");
}

void Ctp2MiniOSpi::OnRspAuthenticate(CThostFtdcRspAuthenticateField* pRspAuthenticateField, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
    if (pRspInfo != NULL && pRspInfo->ErrorID == 0)
    {
        const auto& gConfig = SYSTEM::get_system_config();
        fmt::print("\n====== CTP2MINI Trade Authenticate success ======\n");
        ///用户登录请求
        CThostFtdcReqUserLoginField req = { 0 };
        memset(&req, 0, sizeof(CThostFtdcReqUserLoginField));
        strcpy(req.BrokerID, gConfig.BrokerId.c_str());
        strcpy(req.UserID, gConfig.UserId.c_str());
        strcpy(req.Password, gConfig.PassWord.c_str());

        int iResult = pUserApi->ReqUserLogin(&req, ++m_nRequestID);
        if (iResult != 0)
        {
            fmt::print("\n====== CTP2MINI Trade ReqUserLogin fail ======\n");
        }
    }
    else
    {
        fmt::print("\n====== CTP2MINI Trade Authenticate fail ======\n");

        FILE* fp = nullptr;
        fp = fopen("./errmsg.txt", "w");
        fmt::print(fp, "\n====== CTP2MINI Trade Authenticate fail [ERRORID]:{} [ERRORMSG]:{} ======\n"
            , pRspInfo->ErrorID
            , pRspInfo->ErrorMsg);
        fclose(fp);
    }
}

void Ctp2MiniOSpi::OnRspError(CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
    if (IsErrorRspInfo(pRspInfo)) 
    {
        fmt::print(fp, "\n====== CTP2MINI OnRspError [ERRORID]:{} [ERRORMSG]:{} ======\n"
        , pRspInfo->ErrorID
        , pRspInfo->ErrorMsg);
    }
}

void Ctp2MiniOSpi::OnRspUserLogin(CThostFtdcRspUserLoginField* pRspUserLogin, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
    if (!IsErrorRspInfo(pRspInfo))
    {
        fmt::print("\n====== CTP2MINI Trade UserLogin success ======\n");

        m_nFrontID = pRspUserLogin->FrontID;
        m_nSessionID = pRspUserLogin->SessionID;

        if (m_bIsQryInit || m_bIsSubAllInsts)
        {
            //查询全合约
            CThostFtdcQryInstrumentField req = { 0 };
            memset(&req, 0, sizeof(CThostFtdcQryInstrumentField));
            int iResult = pUserApi->ReqQryInstrument(&req, ++m_nRequestID);
            if (iResult != 0)
            {
                fmt::print("\n====== CTP2MINI Trade ReqQryInstrument fail ======\n");
            }
        }
        else
        {
            const auto& gConfig = SYSTEM::get_system_config();
            //查询资金
            CThostFtdcQryTradingAccountField req = { 0 };
            memset(&req, 0, sizeof(CThostFtdcQryTradingAccountField));
            memcpy(req.BrokerID, gConfig.BrokerId.c_str(), gConfig.BrokerId.length());
            memcpy(req.InvestorID, gConfig.UserId.c_str(), gConfig.UserId.length());
            memcpy(req.CurrencyID, "CNY", strlen("CNY"));
            int iResult = pUserApi->ReqQryTradingAccount(&req, ++m_nRequestID);
            if (iResult != 0)
            {
                fmt::print("\n====== CTP2MINI Trade ReqQryTradingAccount fail ======\n");
            }
        }
    }
    else
    {
        fmt::print("\n====== CTP2MINI Trade UserLogin fail ======\n");

        FILE* fp = nullptr;
        fp = fopen("./errmsg.txt", "w");
        fmt::print(fp, "\n====== CTP2MINI Trade UserLogin fail [ERRORID]:{} [ERRORMSG]:{} ======\n"
            , pRspInfo->ErrorID
            , pRspInfo->ErrorMsg);
        fclose(fp);
    }
}

void Ctp2MiniOSpi::OnRspUserLogout(CThostFtdcUserLogoutField* pUserLogout, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
    if (!IsErrorRspInfo(pRspInfo))
    {
        fmt::print("\n====== CTP2MINI Trade UserLogout success ======\n");
        m_bIsUnInitCompleted = true;
        pUserApi->Release();
        pUserApi = nullptr;
    }
    else
    {
        fmt::print("\n====== CTP2MINI Trade UserLogout fail ======\n");

        FILE* fp = nullptr;
        fp = fopen("./errmsg.txt", "w");
        fmt::print(fp, "\n====== CTP2MINI Trade UserLogout fail [ERRORID]:{} [ERRORMSG]:{} ======\n"
            , pRspInfo->ErrorID
            , pRspInfo->ErrorMsg);
        fclose(fp);
    }
}

void Ctp2MiniOSpi::OnRspQryTradingAccount(CThostFtdcTradingAccountField* pTradingAccount, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
    if (!IsErrorRspInfo(pRspInfo))
    {
        if (pTradingAccount != nullptr)
        {
            fmt::print("\n=== {} ===\nEquity: {:.2f}\nAvailable: {:.2f}\nMargin: {:.2f}\nFee: {:.2f}\n",
                pTradingAccount->AccountID, pTradingAccount->Balance, pTradingAccount->Available, pTradingAccount->CurrMargin, pTradingAccount->Commission
            );
        }
    }
    else
    {
        fmt::print("\n====== CTP2MINI QryTradingAccount fail ======\n");
    }

    if (bIsLast)
    {
        //持仓查询响应
        const auto& gConfig = SYSTEM::get_system_config();
        CThostFtdcQryInvestorPositionField req = { 0 };
        memset(&req, 0, sizeof(CThostFtdcQryInvestorPositionField));
        memcpy(req.BrokerID, gConfig.BrokerId.c_str(), gConfig.BrokerId.length());
        memcpy(req.InvestorID, gConfig.UserId.c_str(), gConfig.UserId.length());
        int iResult = pUserApi->ReqQryInvestorPosition(&req, ++m_nRequestID);
        if (iResult != 0)
        {
            fmt::print("\n====== CTP2MINI Trade ReqQryInvestorPosition fail ======\n");
        }
    }
}

void Ctp2MiniOSpi::OnRspQryInstrument(CThostFtdcInstrumentField* pInstrument, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
    if (!IsErrorRspInfo(pRspInfo))
    {
        if (pInstrument != nullptr)
        {
            std::string inst;
            //对于行情采集流水线默认订阅所有期权和期货的行情
            //对于期货或者期权流水线订阅设置列表中的合约
            switch (m_qryType)
            {
            case QRY_INSTRUMENT_TYPE::FURURE_ONLY:
            {
                if ('1' == pInstrument->ProductClass)
                {
                    if (m_bIsSubAllInsts)
                    {
                        inst = pInstrument->InstrumentID;
                        Insts.push_back(inst);
                    }

                    if (m_bIsQryInit)
                    {
                        auto* pInstsConfig = SYSTEM::find_inst_cfg(pInstrument->InstrumentID);
                        if (pInstsConfig)
                        {
                            pInstsConfig->PriceTick = pInstrument->PriceTick;
                            pInstsConfig->Multiple = pInstrument->VolumeMultiple;
                            fmt::print("[Static] {}: Multiple={}, PriceTick={:.3f}\n", 
                                pInstsConfig->inst.data(), 
                                pInstsConfig->Multiple,
                                pInstsConfig->PriceTick
                            );
                        }
                    }
                    
                }
            }break;
            case QRY_INSTRUMENT_TYPE::OPTION_ONLY:
            {
                if ('2' == pInstrument->ProductClass || '6' == pInstrument->ProductClass)
                {
                    if (m_bIsSubAllInsts)
                    {
                        inst = pInstrument->InstrumentID;
                        Insts.push_back(inst);
                    }
                    if (m_bIsQryInit)
                    {
                        auto* pInstsConfig = SYSTEM::find_inst_cfg(pInstrument->InstrumentID);
                        if (pInstsConfig)
                        {
                            pInstsConfig->PriceTick = pInstrument->PriceTick;
                            pInstsConfig->Multiple = pInstrument->VolumeMultiple;
                            fmt::print("[Static] {}: Multiple={}, PriceTick={:.3f}\n", 
                                pInstsConfig->inst.data(), 
                                pInstsConfig->Multiple,
                                pInstsConfig->PriceTick
                            );
                        }
                    }
                }
            }break;
            case QRY_INSTRUMENT_TYPE::FUTURE_AND_OPTION:
            {
                if ('1' == pInstrument->ProductClass ||
                    '2' == pInstrument->ProductClass ||
                    '6' == pInstrument->ProductClass)
                {
                    if (m_bIsSubAllInsts)
                    {
                        inst = pInstrument->InstrumentID;
                        Insts.push_back(inst);
                    }
                    if (m_bIsQryInit)
                    {
                        auto* pInstsConfig = SYSTEM::find_inst_cfg(pInstrument->InstrumentID);
                        if (pInstsConfig)
                        {
                            pInstsConfig->PriceTick = pInstrument->PriceTick;
                            pInstsConfig->Multiple = pInstrument->VolumeMultiple;
                            fmt::print("[Static] {}: Multiple={}, PriceTick={:.3f}\n", 
                                pInstsConfig->inst.data(), 
                                pInstsConfig->Multiple,
                                pInstsConfig->PriceTick
                            );
                        }
                    }
                }
            }break;
            }
        }
    }
    else
    {
        fmt::print("\n====== CTP2MINI QryInstrument fail ======\n");
    }

    if (bIsLast)
    {
        fmt::print("\n====== CTP2MINI QryInstrument Last Record ======\n");
        if (m_bIsSubAllInsts)
        {
            m_bIsInitCompleted = true;
        }
        else if (m_bIsQryInit)
        {
            const auto& gConfig = SYSTEM::get_system_config();
            //查询手续费率
            CThostFtdcQryInstrumentCommissionRateField req = { 0 };
            memset(&req, 0, sizeof(CThostFtdcQryInstrumentCommissionRateField));
            memcpy(req.BrokerID, gConfig.BrokerId.c_str(), gConfig.BrokerId.length());
            memcpy(req.InvestorID, gConfig.UserId.c_str(), gConfig.UserId.length());
            int iResult = pUserApi->ReqQryInstrumentCommissionRate(&req, ++m_nRequestID);
            if (iResult != 0)
            {
                fmt::print("\n====== CTP2MINI Trade ReqQryInstrumentCommissionRate fail ======\n");
            }
        }
    }
}

void Ctp2MiniOSpi::OnRspQryInstrumentCommissionRate(CThostFtdcInstrumentCommissionRateField* pInstrumentCommissionRate, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
    if (!IsErrorRspInfo(pRspInfo))
    {
        if (pInstrumentCommissionRate != nullptr)
        {
            auto* pInstsConfig = SYSTEM::find_inst_cfg(pInstrumentCommissionRate->InstrumentID);
            if (pInstsConfig)
            {
                //开仓按金额，开仓按手数，平仓按金额，平仓按手数，平今按金额，平今按手数
                pInstsConfig->FeeOpen[0] = pInstrumentCommissionRate->OpenRatioByMoney;
                pInstsConfig->FeeOpen[1] = pInstrumentCommissionRate->OpenRatioByVolume;
                pInstsConfig->FeeClose[0] = pInstrumentCommissionRate->CloseRatioByMoney;
                pInstsConfig->FeeClose[1] = pInstrumentCommissionRate->CloseRatioByVolume;
                pInstsConfig->FeeCloseToday[0] = pInstrumentCommissionRate->CloseTodayRatioByMoney;
                pInstsConfig->FeeCloseToday[1] = pInstrumentCommissionRate->CloseTodayRatioByVolume;
                fmt::print("[FeeRatio] {}: {:.2e},{:.1f},{:.2e},{:.1f},{:.2e},{:.1f}\n", 
                    pInstsConfig->inst.data(),
                    pInstsConfig->FeeOpen[0], pInstsConfig->FeeOpen[1],
                    pInstsConfig->FeeClose[0], pInstsConfig->FeeClose[1],
                    pInstsConfig->FeeCloseToday[0], pInstsConfig->FeeCloseToday[1]
                );
            }
        }
    }
    else
    {
        fmt::print("\n====== CTP2MINI QryInstrumentCommissionRate fail ======\n");
    }

    if (bIsLast)
    {
        fmt::print("\n====== CTP2MINI QryInstrumentCommissionRate Last Record ======\n");
        //持仓查询响应
        if (m_bIsQryInit)
        {
            const auto& gConfig = SYSTEM::get_system_config();
            CThostFtdcQryInvestorPositionField req = { 0 };
            memset(&req, 0, sizeof(CThostFtdcQryInvestorPositionField));
            memcpy(req.BrokerID, gConfig.BrokerId.c_str(), gConfig.BrokerId.length());
            memcpy(req.InvestorID, gConfig.UserId.c_str(), gConfig.UserId.length());
            int iResult = pUserApi->ReqQryInvestorPosition(&req, ++m_nRequestID);
            if (iResult != 0)
            {
                fmt::print("\n====== CTP2MINI Trade ReqQryInvestorPosition fail ======\n");
            }
        }
    }
}

void Ctp2MiniOSpi::OnRspQryInvestorPosition(CThostFtdcInvestorPositionField* pInvestorPosition, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
    if (!IsErrorRspInfo(pRspInfo))
    {
        if (pInvestorPosition != nullptr)
        {
            // 持仓：[今多，昨多，今空，昨空]
            const char* inst = pInvestorPosition->InstrumentID;
            int t = pInvestorPosition->Position;
            int y = pInvestorPosition->YdPosition;
            if (t + y > 0)
            {
                auto iter = inventory.find(inst);
                if (iter == inventory.end()) 
                {
                    inventory[inst] = { 0, 0, 0, 0 };
                }
                switch (pInvestorPosition->PosiDirection)
                {
                case THOST_FTDC_D_Buy:
                {
                    inventory[inst][0] = t;
                    inventory[inst][1] = y;
                }break;
                case THOST_FTDC_D_Sell:
                {
                    inventory[inst][2] = t;
                    inventory[inst][3] = y;
                }break;
                }
            }
        }
    }
    else
    {
        fmt::print("\n====== CTP2MINI QryInvestorPosition fail ======\n");
    }

    if (bIsLast)
    {
        for (const auto& iter : inventory)
        {
            auto* pInstsConfig = SYSTEM::find_inst_cfg(iter->first.data());
            if (pInstsConfig)
            {
                memcpy(pInstsConfig->inventory, iter->second, 16);
            }
            fmt::print("[Inventory] {}: ({}+{},{}+{})\n", iter->first.data(), 
                iter->second[0], iter->second[1],iter->second[2], iter->second[3]
            );
        }
        if (m_bIsQryInit)
        {
            //行情查询
            CThostFtdcQryDepthMarketDataField req = { 0 };
            memset(&req, 0, sizeof(CThostFtdcQryDepthMarketDataField));
            int iResult = pUserApi->ReqQryDepthMarketData(&req, ++m_nRequestID);
            if (iResult != 0)
            {
                fmt::print("\n====== CTP2MINI Trade ReqQryDepthMarketData fail ======\n");
            }
        }
        else
        {
            m_bIsInitCompleted = true;
        }
    }
}

void Ctp2MiniOSpi::OnRspQryDepthMarketData(CThostFtdcDepthMarketDataField* pDepthMarketData, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
    if (!IsErrorRspInfo(pRspInfo))
    {
        if (pDepthMarketData != nullptr)
        {
            auto* pInstsConfig = SYSTEM::find_inst_cfg(pDepthMarketData->InstrumentID);
            if (pInstsConfig)
            {
                pInstsConfig->UpperLimitPrice = pDepthMarketData->UpperLimitPrice;
                pInstsConfig->LowerLimitPrice = pDepthMarketData->LowerLimitPrice;
                fmt::print("[LimitPrice] {}: Lower={:.3f}, Upper={:.3f}\n", 
                    pInstsConfig->inst.data(), 
                    pInstsConfig->LowerLimitPrice,
                    pInstsConfig->UpperLimitPrice
                );
            }
        }
    }
    else
    {
        fmt::print("\n====== CTP2MINI QryDepthMarketData fail ======\n");
    }

    if (bIsLast)
    {
        fmt::print("\n====== CTP2MINI QryDepthMarketData Last Record ======\n");
        m_bIsInitCompleted = true;
    }
}

///报单通知
void Ctp2MiniOSpi::OnRtnOrder(CThostFtdcOrderField* pOrder)
{

#ifdef __LATENCY_TEST
    fmt::print("===OnRtnOrder=== [OrderRef]:{} [OrderSubmitStatus]:{} [OrderStatus]:{} ===\n"
        , pOrder->OrderRef
        , pOrder->OrderSubmitStatus
        , pOrder->OrderStatus);
#endif // __LATENCY_TEST

    if (m_qcbtoa)
    {
        switch (pOrder->OrderStatus)
        {
        //撤单 1
        case THOST_FTDC_OST_Canceled:
        {
            m_qcbtoa->blockPush([&](Queue::CBTOA* cbtoa) {
                cbtoa->reference_id = atoi(pOrder->OrderLocalID);
                cbtoa->msg_type = CALLBACK_TYPE::ORDER_CANCEL;
            });
        } break;
        default:
        { 
            int localid = atoi(pOrder->OrderLocalID);
            int64_t sysid = atoll(pOrder->OrderSysID);
            int64_t& sysorderid = ctp2mini_OrderPool[localid].sysorderid;
            if (sysorderid == -1 and sysid > 0)
            {
                sysorderid = sydid;
                m_qcbtoa->blockPush([&](Queue::CBTOA* cbtoa) {
                    cbtoa->reference_id = localid;
                    cbtoa->msg_type = CALLBACK_TYPE::ORDER_CONFIRM;
                });
            }
        } break;
        }
    }
}

///成交通知
void Ctp2MiniOSpi::OnRtnTrade(CThostFtdcTradeField* pTrade)
{
#ifdef __LATENCY_TEST
    fmt::print("===OnRtnTrade=== [OrderRef]:{} [TradeID]:{} [InstrumentID]:{} [Direction]:{} [OffsetFlag]:{} [Volume]:{} ===\n"
        , pTrade->OrderRef
        , pTrade->TradeID
        , pTrade->InstrumentID
        , pTrade->Direction
        , pTrade->OffsetFlag
        , pTrade->Volume
    );
#endif // __LATENCY_TEST

    if (m_qcbtoa)
    {
        //已完成成交 2
        m_qcbtoa->blockPush([&](Queue::CBTOA* cbtoa) {
            cbtoa->reference_id = atoi(pTrade->OrderLocalID);
            cbtoa->price = pTrade->Price;
            cbtoa->volume = pTrade->Volume;
            cbtoa->msg_type = CALLBACK_TYPE::ORDER_TRADE;
        });
    }

}

///报单录入请求响应
void Ctp2MiniOSpi::OnRspOrderInsert(CThostFtdcInputOrderField* pInputOrder, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
    if (!IsErrorRspInfo(pRspInfo))
    {
#ifdef __LATENCY_TEST
        fmt::print("\n====== CTP2MINI Trade OrderInsert success ======\n");
#endif // __LATENCY_TEST

    }
    else
    {
        m_qcbtoa->blockPush([&](Queue::CBTOA* cbtoa) {
            cbtoa->reference_id = atoi(pInputOrder->OrderRef);
            cbtoa->errid = pRspInfo->ErrorID;
            cbtoa->msg_type = CALLBACK_TYPE::ORDER_ERROR;
        });
        /*if (ERRORID_INSUFFICIENT_MONEY == pRspInfo->ErrorID)
        {
            //因资金不足的错误报单 强平改合约的净头寸
            m_qcbtoa->blockPush([&](Queue::CBTOA* cbtoa) {
                cbtoa->reference_id = atoi(pInputOrder->OrderRef);
                cbtoa->price = pInputOrder->LimitPrice;
                cbtoa->volume = pInputOrder->VolumeTotalOriginal;
                cbtoa->msg_type = CBTOA_MSG_TYPE::ORDER_ERROR_FORCECLOSE;
                });
        }
        else
        {
            //其他错误报单
            m_qcbtoa->blockPush([&](Queue::CBTOA* cbtoa) {
                cbtoa->reference_id = atoi(pInputOrder->OrderRef);
                cbtoa->price = pInputOrder->LimitPrice;
                cbtoa->volume = pInputOrder->VolumeTotalOriginal;
                cbtoa->msg_type = CBTOA_MSG_TYPE::ORDER_ERROR;
                });
        }*/

        //fmt::print("\n====== CTP2MINI Trade OrderInsert fail [ERRORID]:{} [ERRORMSG]:{} ======\n"
        //    , pRspInfo->ErrorID
        //    , pRspInfo->ErrorMsg);

        /*FILE* fp = nullptr;
        fp = fopen("./errmsg.txt", "w");
        fmt::print(fp, "\n====== CTP2MINI Trade OrderInsert fail [ERRORID]:{} [ERRORMSG]:{} ======\n"
            , pRspInfo->ErrorID
            , pRspInfo->ErrorMsg);
        fclose(fp);*/
    }
}

///报单操作请求响应
virtual void OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if (!IsErrorRspInfo(pRspInfo))
    {
#ifdef __LATENCY_TEST
        fmt::print("\n====== CTP2MINI Trade OrderAction success ======\n");
#endif // __LATENCY_TEST

    }
    else
    {
        m_qcbtoa->blockPush([&](Queue::CBTOA* cbtoa) {
            cbtoa->reference_id = atoi(pInputOrder->OrderRef);
            cbtoa->errid = pRspInfo->ErrorID;
            cbtoa->msg_type = CALLBACK_TYPE::ORDER_CANCEL_ERR;
        });
    }
}

std::vector<std::string> Ctp2MiniOSpi::getInsts()
{
    return Insts;
}

