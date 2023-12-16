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
    ///���ͻ����뽻�׺�̨������ͨ������ʱ����δ��¼ǰ�����÷��������á�
    virtual void OnFrontConnected();

    ///��¼������Ӧ
    virtual void OnRspUserLogin(CThostFtdcRspUserLoginField* pRspUserLogin, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

    ///�ǳ�������Ӧ
    virtual void OnRspUserLogout(CThostFtdcUserLogoutField* pUserLogout, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

    ///��������Ӧ��
    virtual void OnRspSubMarketData(CThostFtdcSpecificInstrumentField* pSpecificInstrument, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);
    
    ///�������֪ͨ
    virtual void OnRtnDepthMarketData(CThostFtdcDepthMarketDataField* pDepthMarketData);
};

#endif
#endif