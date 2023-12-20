/**
 * @file          XTFApi.h
 * @brief         API接口头文件
 * @details       API接口头文件
 * @author        xinweiz
 * @date          2022-06-22
 * @version       v4.1
 * @copyright     南京艾科朗克信息科技有限公司
 */

#ifndef XTF_API_H
#define XTF_API_H

#include <unistd.h>

#include "XTFDataStruct.h"
#define XTF_API_VERSION 4.1.760-4b4eb08

class XTF_API_EXPORT XTFSpi {
public:
    virtual ~XTFSpi() = default;

    ///////////////////////////// 生命期管理接口 ////////////////////////////////

    /**
     * API对象启动通知接口
     *
     * 如果API启动后发生连接中断，API会根据配置发起重新连接。
     * 连接成功后，API重新调用onStart接口通知用户，此时isFirstTime为false。
     *
     * 如果API登录后发生连接中断，API重连成功后并不会发起登录请求，需要用户主动发起登录操作。
     *
     * @param errorCode     启动结果。
     *                      0-表示启动成功，可以调用 login() 接口登录柜台；
     *                      X-表示启动失败对应的错误码；
     *
     * @param isFirstTime   是否初次打开。
     *                      可以据此标志位，判断在初次打开时，进行数据的初始化等操作。
     */
    virtual void onStart(int errorCode, bool isFirstTime) {}

    /**
     * API对象停止通知接口
     *
     * @param reason        停止的原因。
     * - ERRXTFAPI_ClosedByUser          : 客户调用stop接口主动关闭
     * - ERRXTFAPI_ClosedByTimeout       : 心跳超时主动关闭
     * - ERRXTFAPI_ClosedBySendError     : 发送数据时，检测到套接字异常，主动关闭
     * - ERRXTFAPI_ClosedByRecvError     : 检测到对端关闭TCP连接
     * - ERRXTFAPI_ClosedByLoginError    : 检测到登录应答协议异常后主动关闭
     * - ERRXTFAPI_ClosedByLogoutError   : 检测到登出应答协议异常后主动关闭
     * - ERRXTFAPI_ClosedByZipStreamError: 检测到流水推送数据异常后主动关闭
     */
    virtual void onStop(int reason) {}

    /**
     * 柜台在交易时段发生重启的通知接口
     *
     * 说明：
     * 1. 柜台在交易时段如果发生重启，API中断后会自动重连；
     * 2. 重新连接后，API会清空上一次登录的所有数据，并重新从柜台加载数据；
     * 3. 由于API的本地数据发生了变化，因此所有外部使用的指针数据会失效，用户需要在收到
     *    onServerReboot事件后，清理上一次的所有数据指针，此刻这些数据指针依然有效；
     *    当onServerReboot事件处理之后，数据指针将会失效。
     */
    virtual void onServerReboot() {}



    ///////////////////////////// 会话管理接口 ////////////////////////////////

    /**
     * 登录结果通知接口
     *
     * @param errorCode     登录结果。
     *                      0-表示登录成功；
     *                      非0-表示发生错误的错误码；
     *                      具体原因请查阅API说明文档或者官方文档平台(http://docs.accelecom.com/)
     *                      关于期货柜台的错误码说明
     *
     * @param exchangeCount 交易所数量，登录成功后返回交易所的可用数量，但此时不能查询交易席位信息。
     *                      需要收到 onReadyForTrading() 或 onLoadFinished() 事件通知时，
     *                      才能查询交易所的交易席位信息。
     */
    virtual void onLogin(int errorCode, int exchangeCount) {}

    /**
     * 登出结果通知接口
     *
     * @param errorCode     登出结果。
     *                      0-表示登出成功；
     *                      非0-表示发生错误的错误码；
     *                      具体原因请查阅API说明文档或者官方文档平台(http://docs.accelecom.com/)
     *                      关于期货柜台的错误码说明
     */
    virtual void onLogout(int errorCode) {}

    /**
     * 修改密码结果通知接口
     *
     * @param errorCode     修改结果。
     *                      0-表示修改成功；
     *                      非0-表示发生错误的错误码；
     *                      具体原因请查阅API说明文档或者官方文档平台(http://docs.accelecom.com/)
     *                      关于期货柜台的错误码说明
     */
    virtual void onChangePassword(int errorCode) {}

    /**
     * 日初静态数据已加载完毕，可进行报单操作。
     *
     * 日初静态数据已经加载完毕，用户可以根据需要查询合约对象，并进行报单；
     * 但是，报单回报会在日内历史流水追平之后，才会到达。
     *
     * 注意，至此阶段日内历史流水数据并未加载完成，暂时无法计算资金和仓位，
     * 如果需要根据资金和仓位进行报单，需要等待流水追平之后，再发送报单。
     *
     * @param account       登录用户账号对象
     *                      在API实例生命周期内，此对象指针一直有效，可以保存使用；
     *                      该对象指针与getAccount()接口查询的一致；
     */
    virtual void onReadyForTrading(const XTFAccount *account) {}

    /**
     * 交易日内所有流水数据已加载完毕
     *
     * 用户可以根据流水数据计算资金和仓位，并据此进行报单。
     *
     * @param account       登录用户账号对象
     *                      在API实例生命周期内，此对象指针一直有效，可以保存使用；
     *                      该对象指针与getAccount()接口查询的一致；
     */
    virtual void onLoadFinished(const XTFAccount *account) {}



    //////////////////////////////// 交易接口 ////////////////////////////////

    /**
     * 报单回报及订单状态改变通知接口
     *
     * IOC订单不成交被交易所撤单时不产生该事件，只产生onCancelOrder事件；成交时会产生onOrder事件和onTrade事件。
     *
     * 说明：
     * 1、该事件在流水重传的时候也会产生,此时isHistory为true;
     * 2、流水数据支持断点续传功能；
     * 3、如果是新创建的API对象，流水数据从头开始推送；
     * 4、如果API对象已存在，连接断开后重连，流水数据从断开处续传推送；
     * @param errorCode     操作错误码
     *                      0-表示成功；非0-表示发生错误的错误码；
     *                      具体原因请查阅API说明文档或者官方文档平台(http://docs.accelecom.com/)
     *                      关于期货柜台的错误码说明
     * @param order         报单回报对象
     *                      XTFOrder对象被创建后，在API实例的生命周期内是一直有效的。
     *                      但是，已成交数量、报单状态等部分字段在接收不同阶段onOrder事件时会更新为不同的值。
     *                      如果需要对报单进行异步处理，那么需要将XTFOrder的字段拷贝到用户管理的内存之后，再做下一步处理。
     *                      否则，XTFOrder对象的可变字段可能被下一次订单状态更新而刷新。
     *
     * 关于订单状态变化的补充说明：
     * 由于交易柜台支持主席平仓的功能，从管理席位接收回报，所以生产环境下会出现订单状态乱序的情况。
     * API如果先收到 (XTF_OS_Queuing, XTF_OS_Canceled, XTF_OS_PartTraded, XTF_OS_AllTraded, XTF_OS_Rejected)
     * 这几种状态中的一种，后收到 XTF_OS_Accepted 状态，将不产生XTF_OS_Accepted状态的事件。
     *
     */
    virtual void onOrder(int errorCode, const XTFOrder *order) {}


    /**
     * 撤单回报通知接口
     *
     * 主动撤单成功或者失败、IOC订单被交易所撤单都会产生该事件，撤单成功时该订单orderStatus为XTF_OS_Canceled
     *
     * 说明：
     * 1、该事件在流水重传的时候也会产生，此时isHistory为true;
     * 2、流水数据支持断点续传功能；
     * 3、如果是新创建的API对象，流水数据从头开始推送；
     * 4、如果API对象已存在，连接断开后重连，流水数据从断开处续传推送；
     *
     * @param errorCode     操作错误码
     *                      0-表示成功；非0-表示发生错误的错误码；
     *                      具体原因请查阅API说明文档或者官方文档平台(http://docs.accelecom.com/)
     *                      关于期货柜台的错误码说明
     * @param cancelOrder   撤单回报对象，同onOrder接口说明
     */
    virtual void onCancelOrder(int errorCode, const XTFOrder *cancelOrder) {}

    /**
     * 成交回报通知接口
     *
     * 说明：
     * 1、该事件在流水重传的时候也会产生,此时isHistory为true;
     * 2、流水数据支持断点续传功能；
     * 3、如果是新创建的API对象，流水数据从头开始推送；
     * 4、如果API对象已存在，连接断开后重连，流水数据从断开处续传推送；
     *
     * @param trade         成交回报对象
     *                      XTFTrade对象被创建后，在API实例的生命周期内是一直有效的。
     *                      虽然XTFTrade对象的字段不再更新，但是关联的XTFOrder对象和成交对应的仓位明细会发生变化。
     *                      如果需要对成交回报进行异步处理，建议将XTFTrade的字段拷贝到用户管理的内存之后，再做下一步处理。
     *                      如果用户不关心报单状态、已成交数量及成交明细，那么也可以直接使用XTFTrade指针。
     */
    virtual void onTrade(const XTFTrade *trade) {}

    /**
     * 行权或对冲报单回报通知接口
     *
     * 说明：
     * 1、该事件在流水重传的时候也会产生,此时isHistory为true;
     * 2、流水数据支持断点续传功能；
     * 3、如果是新创建的API对象，流水数据从头开始推送；
     * 4、如果API对象已存在，连接断开后重连，流水数据从断开处续传推送；
     * @param errorCode     操作错误码
     *                      0-表示成功；非0-表示发生错误的错误码；
     *                      具体原因请查阅API说明文档或者官方文档平台(http://docs.accelecom.com/)
     *                      关于期货柜台的错误码说明
     * @param order         报单回报对象
     *                      XTFOrder对象被创建后，在API实例的生命周期内是一直有效的。
     *
     * 关于订单状态变化的补充说明：
     * 行权或对冲报单回报，没有以下几个状态：
     * - XTF_OS_Queuing
     * - XTF_OS_PartTraded
     * - XTF_OS_AllTraded
     */
    virtual void onExecOrder(int errorCode, const XTFOrder *order) {}


    /**
     * 行权或对冲撤单回报通知接口
     *
     * 主动撤单成功或者失败会产生该事件，撤单成功时该订单orderStatus为XTF_OS_Canceled
     *
     * 说明：
     * 1、该事件在流水重传的时候也会产生，此时isHistory为true;
     * 2、流水数据支持断点续传功能；
     * 3、如果是新创建的API对象，流水数据从头开始推送；
     * 4、如果API对象已存在，连接断开后重连，流水数据从断开处续传推送；
     *
     * @param errorCode     操作错误码
     *                      0-表示成功；非0-表示发生错误的错误码；
     *                      具体原因请查阅API说明文档或者官方文档平台(http://docs.accelecom.com/)
     *                      关于期货柜台的错误码说明
     * @param cancelOrder   撤单回报对象，同onExecOrder接口说明
     */
    virtual void onCancelExecOrder(int errorCode, const XTFOrder *cancelOrder) {}


    /**
     * 持仓组合回报事件通知接口
     *
     * 如果启用自动组合功能，当持仓被组合或解锁组合时，会产生组合回报事件，通过此接口通知用户。
     * API本地没有持久化组合回报事件对象，用户收到事件后，应立即处理对应的持仓组合逻辑。
     *
     * @param errorCode     操作错误码
     *                      0-表示成功；非0-表示发生错误的错误码；
     *                      具体原因请查阅API说明文档或者官方文档平台(http://docs.accelecom.com/)
     *                      关于期货柜台的错误码说明
     * @param combEvent     持仓组合回报事件对象
     *                      该对象API本地没有持久化，不能在本接口之外使用对象指针
     */
    virtual void onPositionCombEvent(int errorCode, const XTFPositionCombEvent &combEvent) {}


    //////////////////////////////// 数据变化通知接口 ////////////////////////////////

    /**
     * 账户发生变化时回调该接口
     *
     * event: 账户变化事件类型
     * - XTF_EVT_AccountCashInOut 账户资金发生出入金流水变化
     *
     * action: 账户变化的动作
     * - XTF_CASH_In 入金
     * - XTF_Cash_Out 出金
     *
     */
    virtual void onAccount(int event, int action, const XTFAccount *account) {}

    /**
     * 交易所前置状态发生变化时回调该接口
     *
     * event:
     * - XTF_EVT_ExchangeChannelConnected
     * - XTF_EVT_ExchangeChannelDisconnected
     */
    virtual void onExchange(int event, int channelID, const XTFExchange *exchange) {}

    /**
     * 合约发生变化时回调该接口
     *
     * event:
     * - XTF_EVT_InstrumentStatusChanged
     */
    virtual void onInstrument(int event, const XTFInstrument *instrument) {}



    //////////////////////////////// 外接行情接口 ////////////////////////////////

    /**
     * 行情发生变化时回调该接口
     *
     * 默认只通知用户subscribe()指定的合约行情，参见subscribe()接口。
     */
    virtual void onBookUpdate(const XTFMarketData *marketData) {}



    //////////////////////////////// 其他通用接口 ////////////////////////////////

    /**
     * 事件通知接口
     *
     * 事件类型：
     *
     * @param event 事件对象
     */
    virtual void onEvent(const XTFEvent &event) {}

    /**
     * 其他错误通知接口
     *
     * 错误说明：
     * - ERRXTFAPI_RetryMaxCount: TCP重连失败且已达最大次数
     * - ERRXTFAPI_SequenceNotContinous: 数据流水序列号不连续
     * - ERRXTFAPI_OrderCreateFailed: 创建报单对象失败
     * - ERRXTFAPI_ExecOrderCreateFailed: 创建行权/对冲报单对象失败
     * - ERRXTFAPI_InstrumentNotFound: 没有找到关联的合约
     * - ERRXTFAPI_UnzipInProgress: 正在处理数据流解压，不能处理新的数据流
     * - ERRXTFAPI_UnzipBufferEmpty: 解压处理缓冲区为空
     * - ERRXTFAPI_UnzipBufferTooLarge: 解压处理缓冲区过大
     * - ERRXTFAPI_UnzipFailed: 解压失败
     *
     * @param errorCode 错误码 具体原因请查阅API说明文档或者官方文档平台(http://docs.accelecom.com/)
     *                        关于期货柜台的错误码说明
     * @param data 附带的错误数据，无数据则为nullptr
     * @param size 错误数据大小，无数据则为0
     */
    virtual void onError(int errorCode, void *data, size_t size) {}
};

class XTF_API_EXPORT XTFApi {
public:
    virtual ~XTFApi() = default;

    ///////////////////////////// 生命期管理接口 ////////////////////////////////

    /**
     * 启动API
     *
     * 调用该接口，API会自动向交易柜台发起连接。
     *
     * 交易柜台的配置信息，默认从创建API对象的配置文件中读取。
     * 也可以通过 setConfig() 接口配置。
     *
     * 如果接口调用成功，将回调 XTFListener::onStart() 接口通知启动的结果。
     *
     * 如果API停止后再重新启动，可以传入新的对象，以处理新的业务逻辑。
     * 例如：
     * XTFSpiA *a = new XTFSpiA()
     * api->start(a);
     * ... // do something.
     * api->stop();
     *
     * XTFSpiB *b = new XTFSpiB()
     * api->start(b); // ok.
     * ... // do something.
     * api->stop();
     *
     * 说明：不建议传入不同的XTFSpi对象，从逻辑处理上存在一定风险。
     *
     * @param spi 回调事件处理对象
     * @return 接口调用成功返回0，否则返回错误码
     */
    virtual int start(XTFSpi *spi) = 0;

    /**
     * 停止API接口
     *
     * 调用该接口，API会断开与交易柜台的连接。
     * 如果接口调用成功，将回调 XTFSpi::onStop() 接口通知停止的结果。
     *
     * 停止后的API接口，可以重新启动。
     * 此时，可以传入新的XTFSpi对象，处理不同的逻辑。
     *
     * @return 接口调用成功返回0，否则返回错误码
     */
    virtual int stop() = 0;



    ///////////////////////////// 会话管理接口 ////////////////////////////////

    /**
     * 登录交易柜台
     *
     * 接口调用成功后，将回调 onLogin() 接口通知登录结果。
     *
     * 登录接口有三种不同形式的重载：
     * 1、不带任何参数的接口，默认使用配置文件中的设置，或者通过 setConfig() 接口设置的信息；
     * 2、仅带有账户和密码的接口，AppID和AuthCode默认使用配置文件的设置。或者通过setConfig() 接口设置的信息；
     * 3、带有全部参数的接口，将会自动覆盖配置文件或 setConfig() 接口设置的信息；
     *
     * @param accountID 资金账户编码
     * @param password  资金账户密码
     * @param appID     应用程序ID
     * @param authCode  认证授权码
     * @return 接口调用成功返回0，否则返回错误码
     */
    virtual int login() = 0;
    virtual int login(const char *accountID, const char *password) = 0;
    virtual int login(const char *accountID, const char *password, const char *appID, const char *authCode) = 0;

    /**
     * 登出交易柜台
     *
     * 该接口调用成功后，将回调 onLogout() 接口通知登录结果。
     *
     * @return 接口调用成功返回0，否则返回错误码
     */
    virtual int logout() = 0;

    /**
     * 修改账户密码
     *
     * 接口调用成功后，将回调 onChangePassword() 接口通知修改密码结果。
     *
     * @return 接口调用成功返回0，否则返回错误码
     *         调用成功并不表示密码修改成功。
     */
    virtual int changePassword(const char *oldPassword, const char *newPassword) = 0;



    //////////////////////////////// 交易接口 ////////////////////////////////

    /**
     * 发送报单
     *
     * 说明：
     * - 该接口默认是线程不安全的，不能在多线程调用同一个API实例对象的报单接口；
     *   如果需要启用线程安全模式，使用setConfig("ORDER_MT_ENABLED", "true")进行配置即可；
     * - 在 XTFSpi::onOrder() 接口中通知用户插入报单的结果和报单状态；
     *
     * @param inputOrder 待插入的报单参数
     * @param inputOrders 待批量插入的报单参数数组
     * @param orderCount 批量插入数量
     * @return 接口调用成功返回0，否则返回错误码
     *         调用成功并不表示报单操作的结果。
     *         ERRXTFAPI_InvalidImp
     *         ERRXTFAPI_NotStarted
     *         ERRXTFAPI_NotLoggedIn
     *         ERRXTFAPI_InvalidInstrument
     *         ERRXTFAPI_OrderCountExceeded
     *         以及网络发送可能出现的错误码
     */
    virtual int insertOrder(const XTFInputOrder &inputOrder) = 0;
    virtual int insertOrders(const XTFInputOrder inputOrders[], size_t orderCount) = 0;

    /**
     * 发送撤单
     *
     * 说明：
     * - 该接口默认是线程不安全的，不能在多线程调用同一个API实例对象的撤单接口；
     *   如果需要启用线程安全模式，使用setConfig("ORDER_MT_ENABLED", "true")进行配置即可；
     * - 如果使用XTFOrder报单对象直接撤单，务必使用API返回的XTFOrder对象指针，
     *   用户不能从外部构造一个XTFOrder来撤单；
     * - 在 XTFSpi::onCancelOrder() 接口中通知撤单的结果和报单状态。
     *
     * @param order 待撤销的报单对象
     * @param orders 待批量撤销的报单对象数组
     * @param orderCount 批量撤单数量
     * @return 接口调用成功返回0，否则返回错误码
     *         调用成功并不表示报单操作的结果。
     *         ERRXTFAPI_InvalidParameter
     *         ERRXTFAPI_InvalidImp
     *         ERRXTFAPI_NotStarted
     *         ERRXTFAPI_NotLoggedIn
     *         ERRXTFAPI_OrderNotFound
     *         ERRXTFAPI_OrderStatusNotAllowedCancel
     *         ERRXTFAPI_OrderCountExceeded
     *         以及网络发送可能出现的错误码
     */
    virtual int cancelOrder(const XTFOrder *order) = 0;
    virtual int cancelOrder(XTFOrderIDType orderIDType, long orderID) = 0;
    virtual int cancelOrders(const XTFOrder *orders[], size_t orderCount) = 0;
    virtual int cancelOrders(XTFOrderIDType orderIDType, long orderIDs[], size_t orderCount) = 0;



    //////////////////////////////// 行权/对冲接口 ////////////////////////////////

    /**
     * 发送行权/对冲报单
     *
     * 说明：
     * - 行权/对冲的本地报单编号与普通订单的本地报单编号，不能冲突，需要进行统一的编号；
     * - 该接口默认是线程不安全的，不能在多线程调用同一个API实例对象的报单接口；
     *   如果需要启用线程安全模式，使用setConfig("ORDER_MT_ENABLED", "true")进行配置即可；
     * - 在 XTFSpi::onExecOrder() 接口中通知用户插入报单的结果和报单状态；
     *
     * @param execOrder 待插入的报单参数
     * @return 接口调用成功返回0，否则返回错误码
     *         调用成功并不表示报单操作的结果。
     *         ERRXTFAPI_InvalidImp
     *         ERRXTFAPI_NotStarted
     *         ERRXTFAPI_NotLoggedIn
     *         ERRXTFAPI_InvalidInstrument
     *         以及网络发送可能出现的错误码
     */
    virtual int insertExecOrder(const XTFExecOrder &execOrder) = 0;

    /**
     * 发送行权/自对冲撤单
     *
     * 说明：
     * - 该接口默认是线程不安全的，不能在多线程调用同一个API实例对象的撤单接口；
     *   如果需要启用线程安全模式，使用setConfig("ORDER_MT_ENABLED", "true")进行配置即可；
     * - 如果使用XTFOrder报单对象直接撤单，务必使用API返回的XTFOrder对象指针，
     *   用户不能从外部构造一个XTFOrder来撤单；
     * - 在 XTFSpi::onCancelExecOrder() 接口中通知撤单的结果和报单状态。
     *
     * @param order 待撤销的报单对象
     * @return 接口调用成功返回0，否则返回错误码
     *         调用成功并不表示报单操作的结果。
     *         ERRXTFAPI_InvalidParameter
     *         ERRXTFAPI_InvalidImp
     *         ERRXTFAPI_NotStarted
     *         ERRXTFAPI_NotLoggedIn
     *         ERRXTFAPI_OrderNotFound
     *         ERRXTFAPI_OrderStatusNotAllowedCancel
     *         以及网络发送可能出现的错误码
     */
    virtual int cancelExecOrder(const XTFOrder *order) = 0;
    virtual int cancelExecOrder(XTFOrderIDType orderIDType, long orderID) = 0;



    //////////////////////////////// 外接行情接口 ////////////////////////////////

    /**
     * 订阅行情
     *
     * 行情订阅成功后，当收到对应合约的行情数据后，会回调 onBookUpdate() 接口。
     *
     * @param instrument    合约对象
     * @return              接口调用成功或失败，调用成功表示行情订阅成功。
     */
    virtual int subscribe(const XTFInstrument *instrument) = 0;

    /**
     * 取消订阅行情
     *
     * @param instrument    合约对象
     * @return              接口调用成功或失败，调用成功表示行情订阅取消成功。
     */
    virtual int unsubscribe(const XTFInstrument *instrument) = 0;

    /**
     * 更新合约行情
     *
     * 暂时仅支持一档行情数据。
     *
     * 更新行情后，API会自动处理以下逻辑：
     * 1、根据合约仓位计算持仓盈亏；
     * 2、如果用户调用subscribe()接口订阅行情数据，会通过 XTFSpi::onBookUpdate() 接口通知用户；
     *
     * @return 接口调用成功或失败，调用成功表示行情更新成功。
     */
    virtual int updateBook(const XTFInstrument *instrument,     ///< 合约对象
                           double lastPrice,                    ///< 最新价
                           double bidPrice,                     ///< 买入价。为零代表无买入价。
                           int    bidVolume,                    ///< 买入量。为零代表无买入量。
                           double askPrice,                     ///< 卖出价。为零代表无卖出价。
                           int    askVolume                     ///< 卖出量。为零代表无卖出量。
    ) = 0;



    //////////////////////////////// 辅助功能接口 ////////////////////////////////

    /**
     * 同步柜台资金
     *
     * 本接口使用同步方式向柜台查询资金。
     *
     * 调用约束：
     * - 本接口不能并发调用，上一次同步请求结束后，才能进行下一次调用；
     * - 调用接口会阻塞调用者线程，直到应答返回或超时返回；
     *
     * 使用建议：
     * - 如果没有外部行情接入，调用此接口可以同步柜台和本地计算的资金差异；
     * - 如果有外部极速行情，建议使用 updateBook() 的方式，在本地计算资金；
     * - 接入外部行情和调用资金同步接口，两种方式不要混用，以免出现资金错误；
     *
     * @param msTimeout 超时时间，单位：毫秒；默认为45毫秒超时，<=0 表示不允许超时
     * @return          0：表示资金同步成功，可以访问XTFAccount对象查看最新的资金数据；
     *                  ETIMEDOUT：表示请求超时；
     *                  <0：表示内部发生错误，可以联系技术支持查看具体错误信息；
     */
    virtual int syncFunds(int msTimeout = 45) = 0;

    /**
     * 传入合约参数，创建一个预热报单，写入用户提供的缓冲区之中。
     *
     * 约束说明：
     * - 发送至柜台的预热报单每秒限制不超过50个，建议发送间隔为：20ms~50ms；
     * - 预热报单和真实报单建议使用同一个线程发送；
     *
     * 默认创建的预热报单没有合约信息，如果需要携带合约信息，传入合约参数即可。
     * 合约可以通过getInstrumentByID()接口查询获得。
     *
     * @param buf           用户提供的预热报单缓冲区
     * @param size          用户提供的预热报单缓冲区大小，不小于64字节。如果大于64字节，那么仅头部的64字节有效。
     * @param instrument    合约指针，默认为空
     * @return              0-表示成功，非0表示对应的错误码
     */
    virtual int buildWarmOrder(void *buf, size_t size, const XTFInstrument *instrument = nullptr) = 0;



    //////////////////////////////// 查询管理接口 ////////////////////////////////

    /**
     * 获取当前资金账户信息
     * @return 返回资金账户对象指针
     */
    virtual const XTFAccount*       getAccount() = 0;

    /**
     * 获取交易所信息
     */
    virtual int                     getExchangeCount() = 0;
    virtual const XTFExchange*      getExchange(int pos) = 0;

    /**
     * 获取合约信息
     */
    virtual int                     getInstrumentCount() = 0;
    virtual const XTFInstrument*    getInstrument(int pos) = 0;
    virtual const XTFInstrument*    getInstrumentByID(const char *instrumentID) = 0;

    /**
     * 获取组合合约信息
     */
    virtual int                     getCombInstrumentCount() = 0;
    virtual const XTFCombInstrument*getCombInstrument(int pos) = 0;
    virtual const XTFCombInstrument*getCombInstrumentByID(const char *combInstrumentID) = 0;

    /**
     * 获取品种信息
     */
    virtual int                     getProductCount() = 0;
    virtual const XTFProduct*       getProduct(int pos) = 0;
    virtual const XTFProduct*       getProductByID(const char *productID) = 0;

    /**
     * 根据指定的过滤条件查询报单
     *
     * 查询所有满足条件的报单可以使用下面的方法：
     * XTFOrderFilter filter;
     * filter.orderStatus[0] = XTF_OS_Queuing;     // 查询正在排队的报单
     * filter.orderStatus[1] = XTF_OS_PartTraded;  // 查询已部分成交的报单
     * ... // set filter conditions.
     *
     * int count = api->findOrders(filter, 0, nullptr);
     * if (count < 0) {
     *     ... // error.
     *     return;
     * }
     *
     * const XTFOrder **orders = new const XTFOrder *[count];
     * int result = api->findOrders(filter, count, orders);
     * if (result < 0) {
     *     ... // error.
     * } else {
     *     ... // find ok.
     * }
     *
     * @param filter 过滤条件
     * @param count  指定最大的查询结果数量
     * @param orders 保存查询结果的缓冲区(数组)
     * @return 实际的查询结果数量，负数表示查询发生错误
     */
    virtual int findOrders(const XTFOrderFilter &filter, unsigned int count, const XTFOrder *orders[]) = 0;

    /**
     * 根据指定的过滤条件查询成交
     *
     * 查询所有满足条件的成交可以使用下面的方法：
     * XTFTradeFilter filter;
     * ... // set filter.
     *
     * int count = api->findTrades(filter, 0, nullptr);
     * const XTFTrade **trades = new const XTFTrade *[count];
     * int result = api->findTrades(filter, count, trades);
     * if (result < 0) {
     *     ... // error.
     * } else {
     *     ... // find ok.
     * }
     *
     * @param filter 过滤条件
     * @param count  指定最大的查询结果数量
     * @param trades 保存查询结果的缓冲区(数组)
     * @return 实际的查询结果数量，负数表示查询发生错误
     */
    virtual int findTrades(const XTFTradeFilter &filter, unsigned int count, const XTFTrade *trades[]) = 0;



    //////////////////////////////// 参数配置管理接口 ////////////////////////////////

    /**
     * 启停仓位自动组合功能
     *
     * 应在启动会话之前设置自动组合功能，启动之后设置不会生效。
     *
     * @param enabled true-启用 false-不启用
     */
    virtual void enableAutoCombinePosition(bool enabled) = 0;

    /**
     * 设置会话参数
     *
     * 应在启动会话之前设置参数，启动之后设置的参数不会生效。
     *
     * 会话参数名称列表：
     * "ACCOUNT_ID"：资金账户ID
     * "ACCOUNT_PWD"：资金账户密码
     * "APP_ID"：应用程序ID
     * "AUTH_CODE"：认证授权码
     * "TRADE_SERVER_IP"：交易服务地址
     * "TRADE_SERVER_PORT"：交易服务端口
     * "QUERY_SERVER_IP"：查询服务地址
     * "QUERY_SERVER_PORT"：查询服务端口
     * "HEARTBEAT_INTERVAL"：心跳间隔时间，单位：毫秒
     * "HEARTBEAT_TIMEOUT"：心跳超时时间，单位：毫秒
     * "TCP_RECONNECT_ENABLED"：TCP断开后是否重连
     * "TCP_RETRY_INTERVAL"：TCP重连最小间隔时间，单位：毫秒
     * "TCP_RETRY_INTERVAL_MAX"：TCP重连最大间隔时间，单位：毫秒
     * "TCP_RETRY_COUNT"：TCP重连次数
     * "TCP_WORKER_CORE_ID"：数据收发线程绑核
     * "TCP_WORKER_BUSY_LOOP_ENABLED"：数据收发线程是否启用BusyLoop模式运行
     * "TRADE_WORKER_CORE_ID"：预热报单线程绑核
     * "TASK_WORKER_CORE_ID"：通用任务线程绑核
     * "POSITION_CALC_ENABLED"：是否计算仓位
     * "MONEY_CALC_ENABLED"：是否计算资金，如果启用资金计算，会默认启用仓位计算
     * "HISTORY_ONLY_CALC_ENABLED"：是否仅计算历史流水的资金和仓位，如果启用，那么历史流水追平后，将不再计算资金和仓位
     * "WARM_INTERVAL"：预热时间间隔，取值范围：[10,50]，单位：毫秒
     * "WARM_ENABLED"：预热报单功能是否启用，取值范围：[true,false]，默认为true：表示启用
     * "ORDER_MT_ENABLED"：是否启用线程安全模式，启用线程安全模式，单个API实例可以支持多个线程同时报单；但是会略微增加报单延时。
     *
     * 用户也可以设置自己的参数，以便在需要的地方使用 getConfig() 接口进行获取。
     * 会话内部不使用这部分用户参数，仅对其临时存储。
     *
     * @param name 参数名称
     * @param value 参数值
     * @return 参数配置成功或失败
     */
    virtual int setConfig(const char *name, const char *value) = 0;

    /**
     * 查询会话参数
     *
     * 参数名称列表： 同上
     *
     * @param name  参数名称，参见setConfig()；
     * @return      参数值
     *              返回的字符串为临时字符串对象，如有需要请保存字符串值，再做后续的处理。
     */
    virtual const char* getConfig(const char *name) = 0;

    /**
     * 获取API版本
     *
     * @return 版本字符串
     */
    virtual const char *getVersion() = 0;
};

#ifdef __cplusplus
extern "C" {
#endif

/**
 * 创建API实例对象
 *
 * 有两种方式创建API实例：
 * 1. 配置文件方式，需要传入配置文件路径。如果传入的路径打开失败，那么创建API实例失败，返回空指针；
 * 2. 参数设置方式，不需要传入配置文件路径。调用函数时配置文件路径参数传入空指针（nullptr）即可。
 *    创建成功后，需要通过API->setConfig()接口设置以下必选参数：
 *    "ACCOUNT_ID"：资金账户ID
 *    "ACCOUNT_PWD"：资金账户密码
 *    "APP_ID"：应用程序ID
 *    "AUTH_CODE"：认证授权码
 *    "TRADE_SERVER_IP"：交易服务地址
 *    "TRADE_SERVER_PORT"：交易服务端口
 *    "QUERY_SERVER_IP"：查询服务地址
 *    "QUERY_SERVER_PORT"：查询服务端口
 *    详细内容请参考API->setConfig()接口注释说明。
 *
 * @param configPath 参数配置文件路径
 * @return API实例对象指针
 */
XTF_API_EXPORT XTFApi*      makeXTFApi(const char *configPath);

/**
 * 查询API版本号
 *
 * @return API版本号字符串
 */
XTF_API_EXPORT const char*  getXTFVersion();

/**
 * 根据错误码查询错误消息描述
 *
 * @param errorCode 错误码
 * @param language  语言类型；0-中文，1-英文
 * @return 错误消息描述
 */
XTF_API_EXPORT const char*  getXTFErrorMessage(int errorCode, int language);

/**
 * 是否启用日志功能
 *
 * @param enabled
 * - true: 启用日志
 * - false: 禁用日志
 */
XTF_API_EXPORT void setXTFLogEnabled(bool enabled);
#ifdef __cplusplus
}
#endif

#endif
