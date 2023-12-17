#ifndef  __CTP2MINIOSPI_H__
#define  __CTP2MINIOSPI_H__

#include "../../include/ctp2mini/ThostFtdcTraderApi.h"
#include "../offerbase.h"

//"INSUFFICIENT_MONEY" value="31" prompt="CTP:�ʽ���"
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
    ///@brief USER_API����
    Queue::qCBTOA* m_qcbtoa;
    CThostFtdcTraderApi* pUserApi;

    std::map<std::string, int[4]> inventory;
    std::vector<std::string> Insts;

    int  m_nRequestID;      //������
    int  m_nFrontID;        //ǰ��ID
    int  m_nSessionID;      //�ỰID

    int m_qryType;
    bool m_bIsQryInit;      //�Ƿ����������ʼʱ�Ĳ�ѯ
    bool m_bIsSubAllInsts;  //�Ƿ���ȫ����Լ������ǣ���Ҫ��ѯȫ��Լ�����ں�������ȫ��Լ���飩

	bool m_bIsInitCompleted;//��ʼ���Ƿ����
	bool m_bIsUnInitCompleted;//����ʼ���Ƿ����

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

    ///�ͻ�����֤��Ӧ
    virtual void OnRspAuthenticate(CThostFtdcRspAuthenticateField* pRspAuthenticateField, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

    virtual void OnRspError(CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

    ///@brief ��¼������Ӧ
    virtual void OnRspUserLogin(CThostFtdcRspUserLoginField* pRspUserLogin, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

    ///�ǳ�������Ӧ
    virtual void OnRspUserLogout(CThostFtdcUserLogoutField* pUserLogout, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

    //�����ѯ��Լ
    virtual void OnRspQryInstrument(CThostFtdcInstrumentField* pInstrument, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

    ///�����ѯ�ʽ��˻���Ӧ
    virtual void OnRspQryTradingAccount(CThostFtdcTradingAccountField* pTradingAccount, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

    ///�����ѯͶ���ֲ߳���Ӧ
    virtual void OnRspQryInvestorPosition(CThostFtdcInvestorPositionField* pInvestorPosition, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

    ///����֪ͨ
    virtual void OnRtnOrder(CThostFtdcOrderField* pOrder);

    ///�ɽ�֪ͨ
    virtual void OnRtnTrade(CThostFtdcTradeField* pTrade);

    ///����¼��������Ӧ
    virtual void OnRspOrderInsert(CThostFtdcInputOrderField* pInputOrder, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	///��������������Ӧ
	virtual void OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    ///�����ѯ��Լ����������Ӧ
    virtual void OnRspQryInstrumentCommissionRate(CThostFtdcInstrumentCommissionRateField* pInstrumentCommissionRate, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

    ///�����ѯ������Ӧ
    virtual void OnRspQryDepthMarketData(CThostFtdcDepthMarketDataField* pDepthMarketData, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);
};

#endif
