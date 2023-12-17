#ifndef  __CTP2MINIOSPI_H__
#define  __CTP2MINIOSPI_H__

#include "../../include/ctp2mini/ThostFtdcTraderApi.h"
#include "../offerbase.h"

//"INSUFFICIENT_MONEY" value="31" prompt="CTP:资金不足"
#define ERRORID_INSUFFICIENT_MONEY 31

struct CTP2MINIOSPI_ORDER_INFO
{
    int64_t sysorderid = -1;
    CThostFtdcInputOrderField InsertReq = { 0 };
    CThostFtdcInputOrderActionField CancelReq = { 0 };
};

class Ctp2MiniOSpi :public CThostFtdcTraderSpi, public OfferBase
{
public:
    Ctp2MiniOSpi(bool isqryinit,bool issuballinsts, int qrytype);
    ~Ctp2MiniOSpi();

private:
    ///@brief USER_API参数
    Queue::qCBTOA* m_qcbtoa;
    CThostFtdcTraderApi* pUserApi;

    std::map<std::string, int[4]> inventory;
    std::vector<std::string> Insts;

    int  m_nRequestID;      //请求编号
    int  m_nFrontID;        //前置ID
    int  m_nSessionID;      //会话ID

    int m_qryType;
    bool m_bIsQryInit;      //是否程序启动初始时的查询
    bool m_bIsSubAllInsts;  //是否订阅全部合约（如果是，需要查询全合约，用于后续订阅全合约行情）

	bool m_bIsInitCompleted;//初始化是否完成
	bool m_bIsUnInitCompleted;//反初始化是否完成

    CTP2MINIOSPI_ORDER_INFO ctp2mini_OrderPool[ORDERPOOL_NUM]{ 0 };

public:
    __attribute__((optimize("O0"))) bool start();
    __attribute__((optimize("O0"))) bool stop();

    std::vector<std::string> getInsts();
    virtual bool send_order(Order& order);
    virtual bool cancel_order(uint32_t id);
    virtual int64_t get_sysorderid(uint32_t);
    virtual void handle_order(int inst_id) { }
private:
    bool IsErrorRspInfo(CThostFtdcRspInfoField* pRspInfo);

public:
    virtual void OnFrontConnected();

    virtual void OnFrontDisconnected(int nReason);

    ///客户端认证响应
    virtual void OnRspAuthenticate(CThostFtdcRspAuthenticateField* pRspAuthenticateField, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

    virtual void OnRspError(CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

    ///@brief 登录请求响应
    virtual void OnRspUserLogin(CThostFtdcRspUserLoginField* pRspUserLogin, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

    ///登出请求响应
    virtual void OnRspUserLogout(CThostFtdcUserLogoutField* pUserLogout, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

    //请求查询合约
    virtual void OnRspQryInstrument(CThostFtdcInstrumentField* pInstrument, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

    ///请求查询资金账户响应
    virtual void OnRspQryTradingAccount(CThostFtdcTradingAccountField* pTradingAccount, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

    ///请求查询投资者持仓响应
    virtual void OnRspQryInvestorPosition(CThostFtdcInvestorPositionField* pInvestorPosition, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

    ///报单通知
    virtual void OnRtnOrder(CThostFtdcOrderField* pOrder);

    ///成交通知
    virtual void OnRtnTrade(CThostFtdcTradeField* pTrade);

    ///报单录入请求响应
    virtual void OnRspOrderInsert(CThostFtdcInputOrderField* pInputOrder, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	///报单操作请求响应
	virtual void OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    ///请求查询合约手续费率响应
    virtual void OnRspQryInstrumentCommissionRate(CThostFtdcInstrumentCommissionRateField* pInstrumentCommissionRate, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

    ///请求查询行情响应
    virtual void OnRspQryDepthMarketData(CThostFtdcDepthMarketDataField* pDepthMarketData, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);
};

#endif
