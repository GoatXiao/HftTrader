#ifndef __XELEOSPI_H__
#define __XELEOSPI_H__

#include "../offerbase.h"
//__ XELE：艾科efvi裸协议报单

#ifdef __OFFER_XELE
#include "../../include/common/Efvi.h"
#include "../../include/xele/XTFApi.h"
#include "../../include/xele/xtf_api_struct.h"

typedef struct alignas(64) 
{
    Order order;
    CXeleFairInputOrderMsg InputOrder;
    CXeleFairOrderActionMsg OrderAction;
} ORDER;

typedef struct alignas(8) {
    //TXeleFtdcInstruIDType instrument;
    TXeleFtdcInstrumentIndexType index;
    TXeleFtdcClientIndexType clientIndex;
    TXeleFtdcClientTokenType clientToken;
} INSTRUMENT;

class XeleOSpi : public XTFSpi, public OfferBase
{
private:
    EfviUdpSender sender;
    uint32_t mRequestID = 0;
    int mSeqNo = 0;
    Queue::qCBTOA* m_qcbtoa;
    HashMap<INSTRUMENT>::type Instruments;
    ORDER Orders[ORDERPOOL_NUM];
    XTFApi* mApi{ nullptr };
    char dummy[64];
    static int64_t LastSendTimeNs;
    static uint32_t LocalID;

    inline void sendData(const uint8_t* data, size_t len);

public:
    void onStart(int errorCode, bool isFirstTime);

    void onLogin(int errorCode, int exchangeCount);

    void onError(int errorCode, void* data, size_t size);

    void onLoadFinished(const XTFAccount* account);

    void onOrder(int errorCode, const XTFOrder* order);

    void onCancelOrder(int errorCode, const XTFOrder* cancelOrder);

    void onTrade(const XTFTrade* trade);

    void onStop(int errorCode) {};
    void onLogout(int errorCode) {};
    void onChangePassword(int errorCode) {};
    void onReadyForTrading(const XTFAccount* account) {};
    void onAccount(int event, int action, const XTFAccount* account) {};
    void onExchange(int event, int channelID, const XTFExchange* exchange) {};
    void onInstrument(int event, const XTFInstrument* instrument) {};
    void onBookUpdate(const XTFMarketData* marketData) {};
    void onEvent(const XTFEvent& event) {};

public:
    XeleOSpi(Queue::qCBTOA* cbtoa);

    ~XeleOSpi();

    virtual bool start();

    virtual bool stop();

    virtual bool send_order(Order& order);

    virtual bool cancel_order(const uint32_t& id);

    virtual std::vector<std::string> getInsts();

    virtual void handle_order(int inst_id) {}

    inline Order& get_order(uint32_t i);

    inline void send_dummy();
};
#endif // XELE

#endif