#ifndef __V10OSPI_H__
#define __V10OSPI_H__

#include "../offerbase.h"

// __V10：易盛efvi裸协议报单

#ifdef __OFFER_V10
#include "../../include/common/Efvi.h"
#include "../../include/v10/DstarTradeApi.h"

static const int header_len = sizeof(DstarApiHead);
static const int delete_len = sizeof(DstarApiReqOrderDeleteField);
static const int insert_len = sizeof(DstarApiReqOrderInsertField);
typedef char OrderInsert[header_len + insert_len];
typedef char OrderDelete[header_len + delete_len];
static const int SIZE_NEW = sizeof(OrderInsert);
static const int SIZE_CXL = sizeof(OrderDelete);
typedef struct alignas(64) 
{
    Order order;
    OrderInsert InputOrder;
    OrderDelete OrderAction;
} ORDER;
class V10OSpi : public IDstarTradeSpi, public OfferBase
{
public:
    V10OSpi(Queue::qCBTOA* cbtoa);
    ~V10OSpi();
private:
    EfviUdpSender sender;
    IDstarTradeApi* m_pApi;
    DstarApiAuthCodeType pUdpAuthCode;
    DstarApiAccountIndexType pAccountIndex;

    Queue::qCBTOA* m_qcbtoa;
    ORDER Orders[ORDERPOOL_NUM];
    StrHash<INSTRUMENTLENGTH, DstarApiContractIndexType, 0, 6> InstNo;

    inline void sendData(const uint8_t* data, size_t len);

public:
    virtual bool start();

    virtual bool stop();

    virtual bool send_order(Order& order);

    virtual bool cancel_order(const uint32_t& id);

    virtual std::vector<std::string> getInsts();

    virtual void handle_order(int inst_id) {}

    inline Order& get_order(uint32_t i);

protected:
    ///报单应答
    virtual void OnRspOrderInsert(const DstarApiRspOrderInsertField* pOrderInsert);

    ///报单通知
    virtual void OnRtnOrder(const DstarApiOrderField* pOrder);

    ///成交通知
    virtual void OnRtnMatch(const DstarApiMatchField* pTrade);

    ///错误应答
    virtual void OnRspError(DstarApiErrorCodeType nErrorCode);

    ///登录请求响应,错误码为0说明用户登录成功。
    virtual void OnRspUserLogin(const DstarApiRspLoginField* pLoginRsp);

    ///合约响应
    virtual void OnRspContract(const DstarApiContractField* pContract);

    ///持仓快照响应
    virtual void OnRspPosition(const DstarApiPositionField* pPosition);

    ///资金快照响应
    virtual void OnRspFund(const DstarApiFundField* pFund);

    ///API准备就绪,只有用户收到此就绪通知时才能进行后续的操作
    virtual void OnApiReady(const DstarApiSerialIdType nSerialId);
    ///UDP认证请求响应,错误码为0说明认证成功。
    virtual void OnRspUdpAuth(const DstarApiRspUdpAuthField* pRspUdpAuth) {};
    ///客户端与通知接口通信连接断开
    virtual void OnFrontDisconnected() {};
    ///系统信息提交响应
    virtual void OnRspSubmitInfo(const DstarApiRspSubmitInfoField* pRspSubmitInfo) {};
    // 密码修改应答
    virtual void OnRspPwdMod(const DstarApiRspPwdModField* pRspPwdModField) {};
    ///持仓快照响应
    virtual void OnRspPrePosition(const DstarApiPrePositionField* pPosition) {};
    ///席位信息响应
    virtual void OnRspSeat(const DstarApiSeatField* pSeat) {};
    ///手续费参数响应
    virtual void OnRspTrdFeeParam(const DstarApiTrdFeeParamField* pFeeParam) {};
    ///保证金参数响应
    virtual void OnRspTrdMarParam(const DstarApiTrdMarParamField* pMarParam) {};
    ///市场状态信息响应
    virtual void OnRspTrdExchangeState(const DstarApiTrdExchangeStateField* pTrdExchangeState) {};
    ///委托响应
    virtual void OnRspOrder(const DstarApiOrderField* pOrder) {};
    ///报价响应
    virtual void OnRspOffer(const DstarApiOfferField* pOffer) {};
    ///成交响应
    virtual void OnRspMatch(const DstarApiMatchField* pTrade) {};
    ///出入金响应
    virtual void OnRspCashInOut(const DstarApiCashInOutField* pCashInOut) {};
    ///撤单应答
    virtual void OnRspOrderDelete(const DstarApiRspOrderDeleteField* pOrderDelete) {};
    ///报价应答
    virtual void OnRspOfferInsert(const DstarApiRspOfferInsertField* pOfferInsert) {};
    ///最新请求号应答
    virtual void OnRspLastReqId(const DstaApiRspLastReqIdField* pLastReqId) {};
    ///出入金通知
    virtual void OnRtnCashInOut(const DstarApiCashInOutField* pCashInOut) {};
    ///报价通知
    virtual void OnRtnOffer(const DstarApiOfferField* pOffer) {};
    ///询价通知
    virtual void OnRtnEnquiry(const DstarApiEnquiryField* pEnquiry) {};
    ///市场状态通知
    virtual void OnRtnTrdExchangeState(const DstarApiTrdExchangeStateField* pTrdExchangeState) {};
    ///浮盈通知
    virtual void OnRtnPosiProfit(const DstarApiPosiProfitField* pPosiProfit) {};
    ///席位信息通知
    virtual void OnRtnSeat(const DstarApiSeatField* pSeat) {};
    // 密码修改通知
    virtual void OnRtnPwdMod(const DstarApiPwdModField* pPwdModField) {};
};
#endif // V10

#endif