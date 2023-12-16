#ifndef  __SIMULATEOSPI_H__
#define  __SIMULATEOSPI_H__

#include "../offerbase.h"

#ifdef __SIMULATE

using InsertOrderBook = std::map<int, Order*>;
using CancelBuffer = std::vector<Order*>;
using InsertBuffer = std::vector<Order*>;

class SimulateOSpi :public OfferBase
{
public:
    SimulateOSpi();
    ~SimulateOSpi();
private:
    Queue::qCBTOA* m_qcbtoa;
    std::vector<MdFeed> m_History;
    InsertOrderBook m_insertorderbook;
    std::vector<CancelBuffer*> m_Cancel;
    std::vector<InsertBuffer*> m_Insert;

    double hit_rate;//非本方挂单
    double place_rate;//本方挂单
public:
    virtual bool start();
    virtual bool stop();
    virtual bool send_order(Order& order);
    virtual bool cancel_order(uint32_t orderid);
    virtual int64_t get_sysorderid(uint32_t orderid);

    //处理报单线程
    void handle_order(int inst_id);

private:
    int handle_deal(Order*, const MdFeed*, const MdFeed*, double, int, int);

    void OnOrderInsert(const Order& order);
    void OnOrderCancel(const Order& order);
    void OnOrderTrade(const Order& order, double price, int volume);
};
#endif
#endif
