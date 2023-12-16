#include "ctp2miniMDSpi.h"

#ifdef __MD_CTP2MINI

Ctp2MiniMDSpi::Ctp2MiniMDSpi()
{
    pUserApi = NULL;
    m_nRequestID = 0;

    ftos_q = nullptr;
    ftol_q = nullptr;
}

Ctp2MiniMDSpi::~Ctp2MiniMDSpi()
{
    pUserApi->Release();
}

bool Ctp2MiniMDSpi::init()
{
    if (pUserApi != nullptr) {
        pUserApi->Release();
    }

    pUserApi = CThostFtdcMdApi::CreateFtdcMdApi(); // 初始化UserApi
    pUserApi->RegisterSpi((CThostFtdcMdSpi*)this);
    pUserApi->RegisterFront(const_cast<char*>(MD_URL));
    pUserApi->Init();

    return true;
}

bool Ctp2MiniMDSpi::uninit()
{
    if (pUserApi)
    {
        CThostFtdcUserLogoutField req = { 0 };
        memset(&req, 0, sizeof(CThostFtdcUserLogoutField));
        int iResult = pUserApi->ReqUserLogout(&req, ++m_nRequestID);
        if (iResult != 0)
        {
            fmt::print("\n====== CTP2MINI MD ReqUserLogout fail ======\n");
            return false;
        }
    }
    return true;
}

void Ctp2MiniMDSpi::setInsts(std::vector<std::string>& _insts)
{
    Insts = _insts;
}

void Ctp2MiniMDSpi::setQueue(PRODUTIONLINE_TYPE plt)
{
    if (PLT_LOGGER == plt) 
    {
        ftos_q = nullptr;
        ftol_q = Queue::get_qftol();
    }
    else
    {
        ftos_q = Queue::get_qftos();
        ftol_q = nullptr;
    }
}

bool Ctp2MiniMDSpi::IsErrorRspInfo(CThostFtdcRspInfoField* pRspInfo)
{
    // 如果ErrorID != 0, 说明收到了错误的响应
    return ((pRspInfo) && (pRspInfo->ErrorID != 0));
}

void Ctp2MiniMDSpi::OnFrontConnected()
{
    fmt::print("\n====== CTP2MINI MD Connected success ======\n");
    ///用户登录请求
    CThostFtdcReqUserLoginField req = { 0 };
    memset(&req, 0, sizeof(CThostFtdcReqUserLoginField));
    memcpy(req.BrokerID, BROKERID, strlen(BROKERID));
    memcpy(req.UserID, USERID, strlen(USERID));
    memcpy(req.Password, PASSWORD, strlen(PASSWORD));

    int iResult = pUserApi->ReqUserLogin(&req, ++m_nRequestID);
    if (iResult != 0)
    {
        fmt::print("\n====== CTP2MINI MD ReqUserLogin fail ======\n");
    }
}

void Ctp2MiniMDSpi::OnRspUserLogin(CThostFtdcRspUserLoginField* pRspUserLogin, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
    if (!IsErrorRspInfo(pRspInfo))
    {
        fmt::print("\n====== CTP2MINI MD UserLogin success ======\n");
        fmt::print("\n====== Insts.size:{} ======\n", Insts.size());

        char** temp = nullptr;
        try {
            temp = new char* [Insts.size() + 1];
            if (temp != nullptr) {
                for (std::size_t i = 0; i < Insts.size(); i++)
                {
                    temp[i] = new char[INSTRUMENTLENGTH];
                    memset(temp[i], 0, INSTRUMENTLENGTH);
                    memcpy(temp[i], Insts[i].c_str(), INSTRUMENTLENGTH);
                }

                int iResult = pUserApi->SubscribeMarketData(temp, Insts.size());
                if (iResult != 0)
                {
                    fmt::print("\n====== CTP2MINI SubscribeMarketData fail ======\n");
                }
            }
            else {
                throw temp;
            }
        }
        catch (...) {
            delete[] temp;
        }
    }
    else
    {
        fmt::print("\n====== CTP2MINI MD OnRspUserLogin fail ======\n");
    }
}

void Ctp2MiniMDSpi::OnRspUserLogout(CThostFtdcUserLogoutField* pUserLogout, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
    if (!IsErrorRspInfo(pRspInfo))
    {
        fmt::print("\n====== CTP2MINI MD UserLogout success ======\n");
        pUserApi->Release();
        pUserApi = nullptr;
    }
    else
    {
        fmt::print("\n====== CTP2MINI MD UserLogout fail ======\n");

        FILE* fp = nullptr;
        fp = fopen("./errmsg.txt", "w");
        fmt::print(fp, "\n====== CTP2MINI MD UserLogout fail [ERRORID]:{} [ERRORMSG]:{} ======\n"
            , pRspInfo->ErrorID
            , pRspInfo->ErrorMsg);
        fclose(fp);
    }
}

///订阅行情应答
void Ctp2MiniMDSpi::OnRspSubMarketData(CThostFtdcSpecificInstrumentField* pSpecificInstrument, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
    if (!IsErrorRspInfo(pRspInfo) && pRspInfo->ErrorID == 0)
    {
        /*fmt::print("\n====== CTP2MINI MD SubMarketData success ======\n");*/
    }
    else
    {
        fmt::print("\n====== CTP2MINI MD [Inst]:{} Subscribe fail ======\n", pSpecificInstrument->InstrumentID);
    }
}

///深度行情通知
void Ctp2MiniMDSpi::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField* pDepthMarketData)
{
    int64_t ns = Timer::tsc();
    if (ftos_q)
    {
        ftos_q->blockPush([&](FEED* feed) {
            feed->iL = 1;
            memcpy(feed->instrument, pDepthMarketData->InstrumentID, INSTRUMENTLENGTH);
            feed->bidvol[0] = pDepthMarketData->BidVolume1;
            feed->bid[0] = pDepthMarketData->BidPrice1;
            feed->askvol[0] = pDepthMarketData->AskVolume1;
            feed->ask[0] = pDepthMarketData->AskPrice1;
            feed->turnover = pDepthMarketData->Turnover;
            feed->openint = pDepthMarketData->OpenInterest;
            feed->volume = pDepthMarketData->Volume;
            feed->price = pDepthMarketData->LastPrice;
            feed->set(pDepthMarketData->UpdateTime, pDepthMarketData->UpdateMillisec);
            feed->ns = ns;
            });
    }

    if (ftol_q)
    {
        const char* p = pDepthMarketData->InstrumentID;
        FEED* _feed = VFEED::get(p);
        if (_feed)
        {
            _feed->iL = 1;
            memcpy(_feed->instrument, pDepthMarketData->InstrumentID, INSTRUMENTLENGTH);
            _feed->bidvol[0] = pDepthMarketData->BidVolume1;
            _feed->bid[0] = pDepthMarketData->BidPrice1;
            _feed->askvol[0] = pDepthMarketData->AskVolume1;
            _feed->ask[0] = pDepthMarketData->AskPrice1;
            _feed->turnover = pDepthMarketData->Turnover;
            _feed->openint = pDepthMarketData->OpenInterest;
            _feed->volume = pDepthMarketData->Volume;
            _feed->price = pDepthMarketData->LastPrice;
            _feed->set(pDepthMarketData->UpdateTime, pDepthMarketData->UpdateMillisec);
            _feed->ns = ns;
        }

        ftol_q->blockPush([&](FEED* feed) {
            feed->iL = 1;
            memcpy(feed->instrument, pDepthMarketData->InstrumentID, INSTRUMENTLENGTH);
            feed->bidvol[0] = pDepthMarketData->BidVolume1;
            feed->bid[0] = pDepthMarketData->BidPrice1;
            feed->askvol[0] = pDepthMarketData->AskVolume1;
            feed->ask[0] = pDepthMarketData->AskPrice1;
            feed->turnover = pDepthMarketData->Turnover;
            feed->openint = pDepthMarketData->OpenInterest;
            feed->volume = pDepthMarketData->Volume;
            feed->price = pDepthMarketData->LastPrice;
            feed->set(pDepthMarketData->UpdateTime, pDepthMarketData->UpdateMillisec);
            feed->ns = ns;
            });

    }
#ifdef __LATENCY_TEST
    //int64_t ns2 = Timer::tsc();
    //fmt::print("[feed.cpp] Latency {} ns\n",
    //    Timer::tsc2ns(ns2) - Timer::tsc2ns(ns)
    //);
#endif
}
#endif