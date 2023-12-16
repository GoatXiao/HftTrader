#ifndef  __CTP2MINIMDSPI_H__
#define  __CTP2MINIMDSPI_H__

#include "../mdbase.h"

#ifdef __MD_CTP2MINI

class Ctp2MiniMDSpi :public CThostFtdcMdSpi, public MDBase
{
public:
    Ctp2MiniMDSpi();
    ~Ctp2MiniMDSpi();
private:
    Queue::qFTOS* ftos_q;
    Queue::qFTOL* ftol_q;
private:
    CThostFtdcMdApi* pUserApi;
    std::vector<std::string> Insts;
public:
    virtual bool init();

    virtual bool uninit();

    virtual void setInsts(std::vector<std::string>& _insts);

    virtual void setQueue(PRODUTIONLINE_TYPE plt);

    bool IsErrorRspInfo(CThostFtdcRspInfoField* pRspInfo);
public:
    ///当客户端与交易后台建立起通信连接时（还未登录前），该方法被调用。
    virtual void OnFrontConnected();

    ///登录请求响应
    virtual void OnRspUserLogin(CThostFtdcRspUserLoginField* pRspUserLogin, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

    ///登出请求响应
    virtual void OnRspUserLogout(CThostFtdcUserLogoutField* pUserLogout, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

    ///订阅行情应答
    virtual void OnRspSubMarketData(CThostFtdcSpecificInstrumentField* pSpecificInstrument, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);
    
    ///深度行情通知
    virtual void OnRtnDepthMarketData(CThostFtdcDepthMarketDataField* pDepthMarketData);
};

#endif
#endif