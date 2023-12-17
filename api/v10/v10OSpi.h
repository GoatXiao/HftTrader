#ifndef __V10OSPI_H__
#define __V10OSPI_H__

#include "../offerbase.h"

// __V10����ʢefvi��Э�鱨��

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
    ///����Ӧ��
    virtual void OnRspOrderInsert(const DstarApiRspOrderInsertField* pOrderInsert);

    ///����֪ͨ
    virtual void OnRtnOrder(const DstarApiOrderField* pOrder);

    ///�ɽ�֪ͨ
    virtual void OnRtnMatch(const DstarApiMatchField* pTrade);

    ///����Ӧ��
    virtual void OnRspError(DstarApiErrorCodeType nErrorCode);

    ///��¼������Ӧ,������Ϊ0˵���û���¼�ɹ���
    virtual void OnRspUserLogin(const DstarApiRspLoginField* pLoginRsp);

    ///��Լ��Ӧ
    virtual void OnRspContract(const DstarApiContractField* pContract);

    ///�ֲֿ�����Ӧ
    virtual void OnRspPosition(const DstarApiPositionField* pPosition);

    ///�ʽ������Ӧ
    virtual void OnRspFund(const DstarApiFundField* pFund);

    ///API׼������,ֻ���û��յ��˾���֪ͨʱ���ܽ��к����Ĳ���
    virtual void OnApiReady(const DstarApiSerialIdType nSerialId);
    ///UDP��֤������Ӧ,������Ϊ0˵����֤�ɹ���
    virtual void OnRspUdpAuth(const DstarApiRspUdpAuthField* pRspUdpAuth) {};
    ///�ͻ�����֪ͨ�ӿ�ͨ�����ӶϿ�
    virtual void OnFrontDisconnected() {};
    ///ϵͳ��Ϣ�ύ��Ӧ
    virtual void OnRspSubmitInfo(const DstarApiRspSubmitInfoField* pRspSubmitInfo) {};
    // �����޸�Ӧ��
    virtual void OnRspPwdMod(const DstarApiRspPwdModField* pRspPwdModField) {};
    ///�ֲֿ�����Ӧ
    virtual void OnRspPrePosition(const DstarApiPrePositionField* pPosition) {};
    ///ϯλ��Ϣ��Ӧ
    virtual void OnRspSeat(const DstarApiSeatField* pSeat) {};
    ///�����Ѳ�����Ӧ
    virtual void OnRspTrdFeeParam(const DstarApiTrdFeeParamField* pFeeParam) {};
    ///��֤�������Ӧ
    virtual void OnRspTrdMarParam(const DstarApiTrdMarParamField* pMarParam) {};
    ///�г�״̬��Ϣ��Ӧ
    virtual void OnRspTrdExchangeState(const DstarApiTrdExchangeStateField* pTrdExchangeState) {};
    ///ί����Ӧ
    virtual void OnRspOrder(const DstarApiOrderField* pOrder) {};
    ///������Ӧ
    virtual void OnRspOffer(const DstarApiOfferField* pOffer) {};
    ///�ɽ���Ӧ
    virtual void OnRspMatch(const DstarApiMatchField* pTrade) {};
    ///�������Ӧ
    virtual void OnRspCashInOut(const DstarApiCashInOutField* pCashInOut) {};
    ///����Ӧ��
    virtual void OnRspOrderDelete(const DstarApiRspOrderDeleteField* pOrderDelete) {};
    ///����Ӧ��
    virtual void OnRspOfferInsert(const DstarApiRspOfferInsertField* pOfferInsert) {};
    ///���������Ӧ��
    virtual void OnRspLastReqId(const DstaApiRspLastReqIdField* pLastReqId) {};
    ///�����֪ͨ
    virtual void OnRtnCashInOut(const DstarApiCashInOutField* pCashInOut) {};
    ///����֪ͨ
    virtual void OnRtnOffer(const DstarApiOfferField* pOffer) {};
    ///ѯ��֪ͨ
    virtual void OnRtnEnquiry(const DstarApiEnquiryField* pEnquiry) {};
    ///�г�״̬֪ͨ
    virtual void OnRtnTrdExchangeState(const DstarApiTrdExchangeStateField* pTrdExchangeState) {};
    ///��ӯ֪ͨ
    virtual void OnRtnPosiProfit(const DstarApiPosiProfitField* pPosiProfit) {};
    ///ϯλ��Ϣ֪ͨ
    virtual void OnRtnSeat(const DstarApiSeatField* pSeat) {};
    // �����޸�֪ͨ
    virtual void OnRtnPwdMod(const DstarApiPwdModField* pPwdModField) {};
};
#endif // V10

#endif