#ifndef __YDOSPI_H__
#define __YDOSPI_H__

#include "../offerbase.h"
// __YD: 易达efvi裸协议报单

#ifdef __OFFER_YD
#include "../../include/common/Efvi.h"
#include "../../include/yd/ydApi.h"

#pragma pack(push, 1)
typedef struct 
{
    char header[16];
    int instrument;
    int8_t direct;
    int8_t offset;
    int8_t speculate;
    int8_t seat;
    double price;
    int volume;
    int localid;
    int8_t type;
    char junk[31];//7
} new_pkt;

typedef struct 
{
    char header[16];
    int orderid = -1;
    int8_t exchange;
    char junk[19];//3
} cc_pkt;
#pragma pack(pop)

typedef struct alignas(8) 
{
    new_pkt InputOrder;
    cc_pkt OrderAction;
} yd_order;

typedef struct alignas(8) 
{
    int ref; 
    int junk;
    int num[4];
} yd_inst;

class YDOSpi : public YDListener, public OfferBase
{
private:
    EfviUdpSender sender;

    YDApi* mApi = nullptr;
    Queue::qCBTOA* m_qcbtoa;
    const YDExchange* m_pExchange;
    
    yd_order ydOrders[ORDERPOOL_NUM];
    bool m_bIsInitCompleted = false;

    std::vector<yd_inst> v_Instruments;
    tsl::robin_map<std::string, yd_inst*> m_Instruments;

    inline bool sendData(const uint8_t* data, size_t len);

public:
    void notifyCaughtUp(void);
    void notifyFinishInit(void);
    void notifyReadyForLogin(bool hasLoginFailed);
    void notifyLogin(int errorNo, int maxOrderRef, bool isMonitor);

    void notifyOrder(const YDOrder* pOrder, const YDInstrument* pInstrument, const YDAccount* pAccount);
    void notifyTrade(const YDTrade* pTrade, const YDInstrument* pInstrument, const YDAccount* pAccount);
    //void notifyFailedOrder(const YDInputOrder* pOrder, const YDInstrument* pInstrument, const YDAccount* pAccount); 
    void notifyFailedCancelOrder(const YDFailedCancelOrder* pFailedOrder, const YDExchange* pExchange, const YDAccount* pAccount);

    void notifyMarketData(const YDMarketData* pMarketData) {};

public:
    YDOSpi();
    ~YDOSpi();

    virtual bool start();
    virtual bool stop();
    virtual bool send_order(Order& order);
    virtual bool cancel_order(uint32_t);
    virtual int64_t get_sysorderid(uint32_t);
    virtual void handle_order(int inst_id) {}
};
#endif // YD
#endif
