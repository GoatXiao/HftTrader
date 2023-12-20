/**
 * @file          XTFDataStruct.h
 * @brief         基础数据结构头文件
 * @details       用于定义基础数据结构包括Account, Exchange, Product, Instrument, MarketData, MarginRate, CommissionRate
 *                全局有且仅有一个Account，表示当前客户
 *                由于支持多个交易所，如SHFE与INE同时使用的情况，所以一个Account关联多个Exchange
 *                每个交易所支持多个品种，所以一个Exchange对应多个Product
 *                每个产品又包含多个合约，所以一个Product对应多个Instrument
 *                每个合约对应一个行情，所以Instrument与MarketData是一一对应的
 *                每个合约对应多头和空头持仓，分别为LongPosition和ShortPosition
 *                每个合约包含自己的保证金率，且不同的投保方式（投机、套利、套保、Internal）对应不同的保证金率
 * @author        xinweiz
 * @date          2022-06-22
 * @version       v4.1
 * @copyright     南京艾科朗克信息科技有限公司
 */

#ifndef XTF_DATA_STRUCT_H
#define XTF_DATA_STRUCT_H

#include <cstdint>

#include "XTFDataType.h"

#if defined(_WIN32)
#ifdef LIB_XTF_API_EXPORT
#   define XTF_API_EXPORT __declspec(dllexport)
#else
#   define XTF_API_EXPORT __declspec(dllimport)
#endif
#else
#   define XTF_API_EXPORT
#endif

#ifdef LIB_XTF_API_EXPORT
#define XTF_CONST
#else
#define XTF_CONST const
#endif

#pragma pack(1)

class XTFExchange;
class XTFAccount;
class XTFPrePosition;
class XTFPosition;
class XTFProduct;
class XTFProductGroup;
class XTFInstrument;
class XTFMarginRatio;
class XTFCommissionRatio;
class XTFInputOrder;
class XTFOrder;
class XTFTrade;
class XTFMarketData;
class XTFCashInOut;
class XTFCombInstrument;
class XTFCombPosition;
class XTFCombPositionDetail;

class XTF_API_EXPORT XTFUserData {
public:
    void           *userPtr;
    double          userDouble1;
    double          userDouble2;
    int             userInt1;
    int             userInt2;

    XTFUserData() {
        userPtr = nullptr;
        userDouble1 = 0.0;
        userDouble2 = 0.0;
        userInt1 = 0;
        userInt2 = 0;
    }
};


/**
 * @brief   客户/账户资金信息对象类
 * @details 客户/账户资金信息
 */
class XTF_API_EXPORT XTFAccount {
public:
    XTFAccountID    accountID;                      ///< 账户编码
    double          preBalance;                     ///< 日初资金
    double          staticBalance;                  ///< 静态权益
    double          deliveryMargin;                 ///< 交割保证金
    double          deposit;                        ///< 今日入金
    double          withdraw;                       ///< 今日出金
    double          margin;                         ///< 占用保证金，期权空头保证金目前支持昨结算价或报单价计算。
    double          frozenMargin;                   ///< 冻结保证金
    double          premium;                        ///< 权利金
    double          frozenPremium;                  ///< 冻结权利金
    double          commission;                     ///< 手续费
    double          frozenCommission;               ///< 冻结手续费
    double          balance;                        ///< 动态权益：静态权益+持仓亏损+平仓盈亏-手续费+权利金
                                                    ///< 其中：
                                                    ///< - 持仓盈亏出现亏损时为负数，计入动态权益与可用资金；
                                                    ///< - 持仓盈亏盈利时为正数，不计入动态权益与可用。
    double          available;                      ///< 可用资金：动态权益-占用保证金-冻结保证金-交割保证金-冻结权利金-冻结手续费
                                                    ///< 其中：占用保证金包含期权与期货
    double          availableRatio;                 ///< 资金可用限度
    double          positionProfit;                 ///< 持仓盈亏，所有合约的持仓盈亏之和
    double          closeProfit;                    ///< 平仓盈亏，所有合约的平仓盈亏之和
    XTFLocalOrderID lastLocalOrderID;               ///< 用户最后一次报单的本地编号
    XTFLocalActionID lastLocalActionID;             ///< 用户最后一次撤单的本地编号
                                                    ///< 如果使用API接口进行撤单，API内部会从(0xF0000001)开始逐步自增生成本地撤单编号。
                                                    ///< 用户可以通过判断最高位来确定是否为自定义的本地撤单编号。
    mutable XTFUserData userData;                   ///< 保留给用户使用的数据对象

    int             getOrderCount() const;          ///< 查询所有报单数量，用于遍历查询所有报单列表
    const XTFOrder* getOrder(int pos) const;        ///< 按位置索引查询报单对象，从0开始计算
    int             getPrePositionCount() const;    ///< 查询所有昨持仓数量，用于遍历查询所有昨持仓列表
    const XTFPrePosition* getPrePosition(int pos) const; ///< 按位置索引查询昨持仓对象，从0开始计算
    int             getPositionCount() const;       ///< 查询所有仓位数量，用于遍历查询所有持仓数据，同一合约的多头和空头是两个不同持仓对象
    const XTFPosition* getPosition(int pos) const;  ///< 按位置索引查询持仓对象，从0开始计算。已平仓合约无法通过此接口查询
    int             getTradeCount() const;          ///< 查询所有成交数量，用于遍历查询所有成交列表
    const XTFTrade* getTrade(int pos) const;        ///< 按位置索引查询成交对象，从0开始计算
    int             getCashInOutCount() const;      ///< 查询所有出入金记录数量，用于遍历查询所有出入金列表
    const XTFCashInOut* getCashInOut(int pos) const;///< 按位置索引查询出入金对象，从0开始计算
    int             getCombPositionCount() const;   ///< 查询所有组合持仓的数量
    const XTFCombPosition* getCombPosition(int pos) const; ///< 按位置索引查询组合持仓对象
};


/**
 * @brief   交易所信息对象类
 * @details 交易所信息
 */
class XTF_API_EXPORT XTFExchange {
public:
    XTFExchangeID       exchangeID;                     ///< 交易所编码，字符串。比如：CFFEX，SHFE，INE，DCE，GFEX，CZCE等
    uint8_t             clientIndex;                    ///< 裸协议报单需要使用该字段，每个交易所对应的值不同。
    uint32_t            clientToken;                    ///< 裸协议报单需要使用该字段，每个交易所对应的值不同。
    XTFDate             tradingDay;                     ///< 交易日，整数字符串。比如：20220915
    bool                hasChannelIP;                   ///< 是否支持席位编号IP地址查询，true表示支持IP地址查询
    mutable XTFUserData userData;                       ///< 保留给用户使用的数据对象

    int                 getChannelCount() const;        ///< 查询所有席位编号数量
    uint8_t             getChannel(int pos) const;      ///< 按位置索引查询席位编号。
                                                        ///< 位置索引pos参数取值范围：[0, getChannelCount() - 1]
                                                        ///< 如果使用API报单直接使用查询的席位编号值即可；
                                                        ///< 如果是裸协议报单，则需要在查询结果的基础上+10作为席位编号。
    const char*         getChannelIP(int pos) const;    ///< 按位置索引查询席位编号IP地址。
                                                        ///< 如果hasChannelIP为false，返回nullptr；
                                                        ///< 否则返回对应的IP地址字符串指针
    const char*         getChannelIPByID(uint8_t id) const; ///< 按席位编号查询IP地址，返回值同getChannelIP()
    int                 getProductGroupCount() const;   ///< 查询品种组数量
    const XTFProductGroup* getProductGroup(int pos) const; ///< 按位置索引查找品种组对象
};


/**
 * @brief   品种组信息对象类
 * @details 品种组信息
 */
class XTF_API_EXPORT XTFProductGroup {
public:
    XTFProductGroupID   productGroupID;                 ///< 品种组代码
    mutable XTFUserData userData;                       ///< 保留给用户使用的数据对象
    int                 getProductCount() const;        ///< 查询品种组包含的品种数量
    const XTFProduct*   getProduct(int pos) const;      ///< 按位置索引查询品种对象
    const XTFExchange*  getExchange() const;            ///< 查询品种组所属的交易所对象
};


/**
 * @brief   品种信息对象类
 * @details 品种信息
 */
class XTF_API_EXPORT XTFProduct {
public:
    XTFProductID        productID;                      ///< 品种代码
    XTFProductClass     productClass;                   ///< 品种类型
    int                 multiple;                       ///< 合约数量乘数
    double              priceTick;                      ///< 最小变动价位
    int                 maxMarketOrderVolume;           ///< 市价报单的最大报单量
    int                 minMarketOrderVolume;           ///< 市价报单的最小报单量
    int                 maxLimitOrderVolume;            ///< 限价报单的最大报单量
    int                 minLimitOrderVolume;            ///< 限价报单的最小报单量
    mutable XTFUserData userData;                       ///< 保留给用户使用的数据对象
    int                 getInstrumentCount() const;     ///< 查询品种包含的合约数量
    const XTFInstrument* getInstrument(int pos) const;  ///< 按位置索引查询合约对象
    const XTFProductGroup* getProductGroup() const;     ///< 查询品种所属的品种组对象
};


/**
 * @brief   合约对象类
 * @details 合约
 */
class XTF_API_EXPORT XTFInstrument {
public:
    XTFInstrumentID         instrumentID;                   ///< 合约代码
    uint32_t                instrumentIndex;                ///< 合约序号
    int                     deliveryYear;                   ///< 交割年份
    int                     deliveryMonth;                  ///< 交割月份
    int                     maxMarketOrderVolume;           ///< 市价报单的最大报单量
    int                     minMarketOrderVolume;           ///< 市价报单的最小报单量
    int                     maxLimitOrderVolume;            ///< 限价报单的最大报单量
    int                     minLimitOrderVolume;            ///< 限价报单的最小报单量
    double                  priceTick;                      ///< 最小变动价位
    int                     multiple;                       ///< 合约数量乘数
    XTFOptionsType          optionsType;                    ///< 期权类型
    XTFDate                 expireDate;                     ///< 合约到期日，字符串格式：20220915
    bool                    singleSideMargin;               ///< 是否单边计算保证金
    XTFInstrumentStatus     status;                         ///< 当前状态
    mutable XTFUserData     userData;                       ///< 保留给用户使用的数据对象

    const XTFExchange*      getExchange() const;            ///< 查询合约所属的XTFExchange指针
    const XTFProduct*       getProduct() const;             ///< 查询合约所属的XTFProduct指针
    const XTFMarginRatio*   getMarginRatio(XTFHedgeFlag hedgeFlag = XTF_HF_Speculation) const; ///< 保证金率
    const XTFCommissionRatio* getCommissionRatio(XTFHedgeFlag hedgeFlag = XTF_HF_Speculation) const; ///< 手续费率
    const XTFPrePosition*   getPrePosition(XTFHedgeFlag hedgeFlag = XTF_HF_Speculation) const; ///< 历史持仓，来自日初数据，交易日内不会变化
    const XTFPosition*      getLongPosition() const;        ///< 查询合约当前多头持仓，包含历史持仓数据
    const XTFPosition*      getShortPosition() const;       ///< 查询合约当前空头持仓，包含历史持仓数据
    const XTFMarketData*    getMarketData() const;          ///< 查询合约的行情XTFMarketData指针
    const XTFInstrument*    getUnderlyingInstrument() const;///< 如果是期权合约，表示对应的基础合约指针
};


/**
 * @brief   客户日初持仓对象类
 * @details 客户日初持仓
 */
class XTF_API_EXPORT XTFPrePosition {
public:
    int                     preLongPosition;        ///< 昨多头持仓量
    int                     preShortPosition;       ///< 昨空头持仓量
    double                  preSettlementPrice;     ///< 昨结算价
    mutable XTFUserData     userData;               ///< 保留给用户使用的数据对象
    const XTFInstrument*    getInstrument() const;  ///< 查询昨持仓所属合约XTFInstrument的指针
};


class XTF_API_EXPORT XTFTradeDetail {
public:
    XTF_CONST XTFTrade     *trade;                  ///< 成交对象
    int                     volume;                 ///< 成交量
};


///< 定义持仓明细结构体
class XTF_API_EXPORT XTFPositionDetail {
public:
    XTF_CONST XTFTrade     *openTrade;              ///< 开仓成交回报，当等于nullptr时，代表昨仓，昨仓只会出现在持仓明细链表的头部
    XTFTradeID              openTradeID;            ///< 开仓成交编码
    double                  openPrice;              ///< 开仓成交价格，与成交回报中的价格相同。如果是历史持仓则表示昨结算价。
    int                     openVolume;             ///< 持仓明细开仓数量
    int                     remainingVolume;        ///< 持仓明细中剩余仓位总数量
    int                     getCloseTradeCount() const;         ///< 平仓成交回报数量
    const XTFTradeDetail&   getCloseTradeDetail(int pos) const; ///< 平仓成交回报明细
    bool                    isHistory() const { return openTrade == nullptr; } ///< 判断是否为历史持仓
    int                     getCombPositionDetailCount() const;         ///< 持仓明细关联的组合仓位明细列表
    const XTFCombPositionDetail& getCombPositionDetail(int pos) const;  ///< 持仓明细
    int                     getCombinedVolume() const;                  ///< 持仓明细中被组合占用的数量
};


class XTF_API_EXPORT XTFPosition {
public:
    XTFPositionDirection    direction;              ///< 持仓方向：多头、空头
    int                     position;               ///< 持仓总量：历史持仓 - 已平历史持仓 + 今仓 - 已平今仓，包含了平仓冻结量
    int                     todayPosition;          ///< 今持仓量：今仓 - 已平今仓，包含了平仓冻结量（今仓部分）
    int                     combinedPosition;       ///< 已组合的持仓数量
    int                     openFrozen;             ///< 开仓冻结量
    int                     closeFrozen;            ///< 平仓冻结量
    double                  margin;                 ///< 占用保证金，表示持仓的占用保证金。不考虑单向大边或组合持仓的保证金减免逻辑。
    double                  paidMargin;             ///< 实付保证金，表示实际支付的占用保证金。在计算单向大边或启用组合时，实付保证金可能会小于占用保证金。
    double                  frozenMargin;           ///< 冻结保证金
    double                  frozenCommission;       ///< 冻结手续费
    double                  totalOpenPrice;         ///< 总开仓金额：昨仓使用昨结算价，今仓使用成交价计算
    double                  positionProfit;         ///< 持仓盈亏 本合约剩余仓位的持仓盈亏（通过现价计算的动态值）
    double                  closeProfit;            ///< 平仓盈亏 本合约所有今日已平仓位计算的总盈亏（静态值）
    mutable XTFUserData     userData;               ///< 保留给用户使用的数据对象

    double          getOpenPrice() const;           ///< 持仓均价
    int             getAvailablePosition() const;   ///< 获取可用仓位（持仓总量 - 平仓冻结量）
    int             getYesterdayPosition() const;   ///< 获取剩余的历史持仓（历史持仓总量 - 已平历史持仓量）
    int             getPositionDetailCount() const; ///< 查询仓位明细数量
    const XTFPositionDetail& getPositionDetail(int pos) const; ///< 查询仓位明细，包括开仓成交和平仓成交明细
    const XTFInstrument* getInstrument() const;     ///< 所属XTFInstrument的指针
};

class XTF_API_EXPORT XTFCashInOut {
public:
    XTFCashDirection        direction;              ///< 出入金方向
    double                  amount;                 ///< 出入金金额
    XTFTime                 time;                   ///< 出入金时间
    mutable XTFUserData     userData;               ///< 保留给用户使用的数据对象
};


/**
 * @brief   报单请求对象类
 * @details 报单请求
 */
class XTF_API_EXPORT XTFInputOrder {
public:
    XTFLocalOrderID         localOrderID;           ///< 本地报单编号（用户）
                                                    ///< 本地报单编号需要由用户保证唯一性，用于本地存储索引。
                                                    ///< 注意不能与柜台保留的几个特殊ID冲突：
                                                    ///< 1. 非本柜台报单固定为0x88888888；
                                                    ///< 2. 柜台清流启动后的历史报单固定为0xd8888888；
                                                    ///< 3. 柜台平仓报单固定为0xe8888888；
                                                    ///< 为保证报单性能，API不做本地报单编号重复的校验。
                                                    ///< 如果API发生了断线重连，在历史流水追平之后，请继续保持后续本地报单编号与历史报单编号的唯一性。
                                                    ///< 如果不能保证本地报单编号的唯一性，请不要使用API的订单管理功能。
                                                    ///< API允许客户端使用同一用户名/口令多次登录，但客户端需要使用某种机制确保本地报单编号不发生重复。
                                                    ///< 比如：（奇偶交替、分割号段）+单向递增。

    XTFDirection            direction;              ///< 买卖方向
    XTFOffsetFlag           offsetFlag;             ///< 开平仓标志
    XTFOrderType            orderType;              ///< 报单类型：限价(GFD)/市价/FAK/FOK
    double                  price;                  ///< 报单价格
    uint32_t                volume;                 ///< 报单数量
    uint32_t                minVolume;              ///< 最小成交数量。当报单类型为FAK时，
                                                    ///< 如果 minVolume > 1，那么API默认使用最小成交数量进行报单；
                                                    ///< 如果 minVolume ≤ 0，那么API默认使用任意成交数量进行报单；
    XTFChannelSelectionType channelSelectionType;   ///< 席位编号选择类型
    uint8_t                 channelID;              ///< 席位编号
    XTFOrderFlag            orderFlag;              ///< 报单标志（不使用，默认都是普通报单）

    XTF_CONST XTFInstrument *instrument;            ///< 报单合约对象
};


/**
 * @brief   报单请求对象类
 * @details 报单请求
 */
class XTF_API_EXPORT XTFExecOrder {
public:
    XTFLocalOrderID         localOrderID;           ///< 本地报单编号，需要由用户保证唯一性，用于本地存储索引。
                                                    ///< 不能和XTFInputOrder本地报单编号发送冲突，应与XTFInputOrder的本地报单编号统一处理。
    XTFOrderFlag            orderFlag;              ///< 报单标志，用于区分是行权还是自对冲
    XTFOrderType            orderType;              ///< 报单类型：
                                                    ///< - 行权：XTF_ODT_SelfClose | XTF_ODT_NotSelfClose
                                                    ///< - 对冲：XTF_ODT_SelfCloseOptions | XTF_ODT_SelfCloseFutures
    XTFOffsetFlag           offsetFlag;             ///< 开平仓标志：XTF_OF_Close | XTF_OF_CloseToday | XTF_OF_CloseYesterday
    XTFDirection            direction;              ///< 行权和对冲方向：
                                                    ///< - XTF_D_Buy：请求行权、请求对冲；
                                                    ///< - XTF_D_Sell：放弃行权、请求不对冲；
    XTFHedgeFlag            hedgeFlag;              ///< 投机套保标志
    double                  minProfit;              ///< 行权最小利润
    uint16_t                volume;                 ///< 行权数量
    XTFChannelSelectionType channelSelectionType;   ///< 席位编号选择类型
    uint8_t                 channelID;              ///< 席位编号
    XTF_CONST XTFInstrument *instrument;            ///< 报单合约对象
};


/**
 * @brief   报单回报对象类
 * @details 报单回报定义
 */
class XTF_API_EXPORT XTFOrder {
public:
    XTFSysOrderID           sysOrderID;             ///< 柜台流水号
    XTFLocalOrderID         localOrderID;           ///< 用户填写的本地报单号，必须保证唯一性，否则会产生回报错误
                                                    ///< 错误为1172“请求中的报单编号不存在”时，返回0，其他情况返回订单真实编号
                                                    ///< 保留的特殊本地单号：
                                                    ///< 1. 非本柜台报单固定为0x88888888；
                                                    ///< 2. 柜台清流启动后的历史报单固定为0xd8888888；
                                                    ///< 3. 柜台平仓报单固定为0xe8888888；
    XTFExchangeOrderID      exchangeOrderID;        ///< 交易所报单编号
    XTFDirection            direction;              ///< 买卖方向
    XTFOffsetFlag           offsetFlag;             ///< 开平仓标志
    double                  orderPrice;             ///< 报单价格
    uint32_t                orderVolume;            ///< 报单数量
    uint32_t                orderMinVolume;         ///< 最小成交数量
    uint32_t                totalTradedVolume;      ///< 报单累计已成交数量
    XTFOrderType            orderType;              ///< 限价/FAK/市价类型
    XTFOrderFlag            orderFlag;              ///< 报单标志
    XTFChannelSelectionType channelSelectionType;   ///< 席位连接选择
    uint8_t                 channelID;              ///< 席位连接编号，0xFF表示无效值
    uint8_t                 realChannelID;          ///< 实际席位连接编号，由柜台返回，0xFF表示无效值
    XTFOrderStatus          orderStatus;            ///< 报单状态
    XTFDate                 insertDate;             ///< 报单插入日期，字符串格式：20220915
    XTFTime                 insertTime;             ///< 报单插入时间，字符串格式：10:20:30
    XTFTime                 updateTime;             ///< 报单更新时间，字符串格式：10:20:30
    XTFTime                 cancelTime;             ///< 报单撤单时间，字符串格式：10:20:30
    bool                    isHistory;              ///< 回报链路断开重连后或者程序重启后，客户端API会自动进行流水重构，
                                                    ///< 在追平服务器流水之前收到的报单回报，该字段为true。追平流水之后，该字段为false。
                                                    ///< 如对流水重构的回报不需要特殊处理，可不用处理该字段。
    bool                    isSuspended;            ///< 报单是否挂起（暂未使用）
    XTF_CONST XTFInstrument *instrument;            ///< 所属XTFInstrument的指针。如果报单传入的合约不存在，那么合约对象指针可能为空。
    XTFOrderActionType      actionType;             ///< 报单对象对应的操作类型（比如：报单、撤单、挂起、激活等），只读字段，外部调用不需要设置该字段。
    mutable XTFUserData     userData;               ///< 保留给用户使用的数据对象
    int                     getTradeCount() const;  ///< 查询报单对应的成交明细数量
    const XTFTrade*         getTrade(int pos) const;///< 按位置索引查询报单对应的成交明细

    XTFHedgeFlag            getHedgeFlag() const;
    double                  getOptionsExecMinProfit() const;
    XTFOptionsExecResult    getOptionsExecResult() const;
};


/**
 * @brief   成交回报对象类
 * @details 成交回报定义
 */
class XTF_API_EXPORT XTFTrade {
public:
    XTFTradeID              tradeID;                ///< 交易所成交编码
    double                  tradePrice;             ///< 成交价格
    uint32_t                tradeVolume;            ///< 本次回报已成交数量
    double                  margin;                 ///< 该字段已废弃
    double                  commission;             ///< 本次回报已成交手数产生的手续费
    XTFTime                 tradeTime;              ///< 报单成交时间，字符串格式：10:20:30
    XTFDirection            direction;              ///< 买卖方向,详情参考XTFDataType.h
    XTFOffsetFlag           offsetFlag;             ///< 开平仓标志,详情参考XTFDataType.h
    bool                    isHistory;              ///< 回报链路断开重连后或者程序重启后，客户端API会自动进行流水重构，
                                                    ///< 在追平服务器流水之前收到的成交回报，该字段为true。追平流水之后，该字段为false。
                                                    ///< 如对流水重构的回报不需要特殊处理，可不用处理该字段。

    XTF_CONST XTFOrder     *order;                  ///< 所属XTFOrder的指针
    XTF_CONST XTFTrade     *matchTrade;             ///< 对手成交回报，如果不为空则表明自成交
    mutable XTFUserData     userData;               ///< 保留给用户使用的数据对象

    bool                    isSelfTraded() const { return matchTrade != nullptr; }  ///< 判断是否为自成交
};


/**
 * @brief   保证金率对象类
 * @details 保证金率定义
 */
class XTF_API_EXPORT XTFMarginRatio {
public:
    double                  longMarginRatioByMoney;     ///< 按照金额计算的多头保证金率。
    double                  longMarginRatioByVolume;    ///< 按照数量计算的多头保证金率。
    double                  shortMarginRatioByMoney;    ///< 按照金额计算的空头保证金率。
    double                  shortMarginRatioByVolume;   ///< 按照数量计算的空头保证金率。
    mutable XTFUserData     userData;                   ///< 保留给用户使用的数据对象
    const XTFInstrument*    getInstrument() const;      ///< 所属XTFInstrument的指针
};


/**
 * @brief   手续费率对象类
 * @details 手续费率定义
 */
class XTF_API_EXPORT XTFCommissionRatio {
public:
    double                  openRatioByMoney;           ///< 按金额计算的开仓手续费率
    double                  openRatioByVolume;          ///< 按数量计算的开仓手续费率
    double                  closeRatioByMoney;          ///< 按金额计算的平昨手续费率
    double                  closeRatioByVolume;         ///< 按数量计算的平昨手续费率
    double                  closeTodayRatioByMoney;     ///< 按金额计算的平今手续费率
    double                  closeTodayRatioByVolume;    ///< 按数量计算的平今手续费率
    mutable XTFUserData     userData;                   ///< 保留给用户使用的数据对象
    const XTFInstrument*    getInstrument() const;      ///< 所属XTFInstrument的指针
};


/**
 * @brief   行情信息对象类
 * @details 行情信息
 */
class XTF_API_EXPORT XTFMarketData {
public:
    int             tradingDay;                     ///< 交易日
    double          preSettlementPrice;             ///< 前结算价
    double          preClosePrice;                  ///< 前收盘价
    double          preOpenInterest;                ///< 前持仓量（暂未使用）
    double          upperLimitPrice;                ///< 涨停价
    double          lowerLimitPrice;                ///< 跌停价
    double          lastPrice;                      ///< 最新价（接入外部行情后有效）
    double          bidPrice;                       ///< 买入价。为零代表无买入价（接入外部行情后有效）
    int             bidVolume;                      ///< 买入量。为零代表无买入价（接入外部行情后有效）
    double          askPrice;                       ///< 卖出价。为零代表无卖出价（接入外部行情后有效）
    int             askVolume;                      ///< 卖出量。为零代表无卖出价（接入外部行情后有效）
    int             volume;                         ///< 成交量（暂未使用）
    int             snapTime;                       ///< 时间戳（暂未使用）
    double          turnover;                       ///< 成交金额（暂未使用）
    double          openInterest;                   ///< 持仓量（暂未使用）
    double          averagePrice;                   ///< 行情均价（暂未使用）
    mutable XTFUserData userData;                   ///< 保留给用户使用的数据对象

    const XTFInstrument* getInstrument() const;     ///< 所属XTFInstrument的指针
};


////////////////////////////////////////////////////////////////////////////////
// 事件通知定义

class XTF_API_EXPORT XTFEvent {
public:
    char                tradingDay[9];                  ///< 通知事件日期(交易日)
    char                eventTime[9];                   ///< 通知事件时间
    XTFEventType        eventType;                      ///< 通知事件类型
    XTFEventID          eventID;                        ///< 通知事件ID
    int                 eventLen;                       ///< 通知事件数据长度
    char                eventData[256];                 ///< 通知事件的数据
    char                reserve[2];                     ///< 预留字段
};
static_assert(sizeof(XTFEvent) == 288, "XTFEvent size error.");

/// 投资者风控事件通知结构体
class XTF_API_EXPORT XTFPrivateEventInvestorPrc {
public:
    XTFPrcID            prcID;                          ///< 风控的具体ID类型
    XTFAccountID        accountID;                      ///< 投资者名称
    int                 prcValue;                       ///< 投资者风控值
    char                reserve[4];                     ///< 预留字段
};
static_assert(sizeof(XTFPrivateEventInvestorPrc) == 32, "XTFPrivateEventInvestorPrc size error.");

/// 投资者合约风控事件通知结构体
class XTF_API_EXPORT XTFPrivateEventInstrumentPrc {
public:
    XTFPrcID            prcID;                          ///< 风控的具体ID类型
    XTFAccountID        accountID;                      ///< 投资者名称
    char                instrumentID[31];               ///< 合约名称
    int                 prcValue;                       ///< 合约风控值
    char                reserve[5];                     ///< 预留字段
};
static_assert(sizeof(XTFPrivateEventInstrumentPrc) == 64, "XTFPrivateEventInstrumentPrc size error.");


/**
 * @brief   查询持仓信息的过滤器对象类（暂未使用）
 * @details 查询持仓信息的过滤器定义
 */
class XTF_API_EXPORT XTFPositionFilter {
public:
    int                  positionDate;          ///< 持仓日期
    XTFPositionDirection positionDirection;     ///< 持仓方向
    XTFHedgeFlag         hedgeFlag;             ///< 套保标志，-1表示所有
    XTF_CONST XTFExchange *exchange;            ///< 指向交易所结构的指针，空指针表示所有
    XTF_CONST XTFProduct  *product;             ///< 指向品种结构的指针，空指针表示所有
};


/**
 * @brief   查询报单信息的过滤器对象类
 * @details 查询报单信息的过滤器定义
 */
class XTF_API_EXPORT XTFOrderFilter {
public:
    XTFTime             startTime;          ///< 开始时间，字符串格式：10:20:30，空字符串表示所有
    XTFTime             endTime;            ///< 结束时间，字符串格式：10:20:30，空字符串表示所有
    XTFDirection        direction;          ///< 指定报单买卖方向，无效值表示所有
    XTFOffsetFlag       offsetFlag;         ///< 指定报单开平标志，无效值表示所有
    XTFOrderFlag        orderFlag;          ///< 指定报单标志，无效值表示所有
    XTFOrderType        orderType;          ///< 指定报单指令类型，无效值表示所有
    XTFOrderStatus      orderStatus[4];     ///< 指定报单状态，全部为无效值时表示所有，支持同时查询四种状态
    XTF_CONST XTFInstrument *instrument;    ///< 指向合约结构的指针，空指针表示所有
    XTF_CONST XTFProduct    *product;       ///< 指向品种结构的指针，空指针表示所有
    XTF_CONST XTFExchange   *exchange;      ///< 指向交易所结构的指针，空指针表示所有

    XTFOrderFilter() {
        startTime[0] = '\0';
        endTime[0] = '\0';
        direction = XTF_D_Invalid;
        offsetFlag = XTF_OF_Invalid;
        orderFlag = XTF_ODF_Invalid;
        orderType = XTF_ODT_Invalid;
        orderStatus[0] = XTF_OS_Invalid;
        orderStatus[1] = XTF_OS_Invalid;
        orderStatus[2] = XTF_OS_Invalid;
        orderStatus[3] = XTF_OS_Invalid;
        instrument = nullptr;
        product = nullptr;
        exchange = nullptr;
    }
};


/**
 * @brief   查询报单信息的过滤器对象类
 * @details 查询报单信息的过滤器定义
 */
class XTF_API_EXPORT XTFTradeFilter {
public:
    XTFTime             startTime;          ///< 开始时间，字符串格式：10:20:30，空字符串表示所有
    XTFTime             endTime;            ///< 结束时间，字符串格式：10:20:30，空字符串表示所有
    XTFDirection        direction;          ///< 指定报单买卖方向，无效值表示所有
    XTFOffsetFlag       offsetFlag;         ///< 指定报单开平标志，无效值表示所有
    XTF_CONST XTFInstrument *instrument;    ///< 指向合约结构的指针，空指针表示所有
    XTF_CONST XTFProduct    *product;       ///< 指向品种结构的指针，空指针表示所有
    XTF_CONST XTFExchange   *exchange;      ///< 指向交易所结构的指针，空指针表示所有

    XTFTradeFilter() {
        startTime[0] = '\0';
        endTime[0] = '\0';
        direction = XTF_D_Invalid;
        offsetFlag = XTF_OF_Invalid;
        instrument = nullptr;
        product = nullptr;
        exchange = nullptr;
    }
};


class XTF_API_EXPORT XTFCombInstrument {
public:
    XTFInstrumentID         combInstrumentID;               ///< 组合合约编号
    uint32_t                combInstrumentIndex;            ///< 组合合约序号
    XTFCombType             combType;                       ///< 组合类型
    XTFCombDirection        combDirection;                  ///< 组合合约方向
    mutable XTFUserData     userData;                       ///< 保留给用户使用的数据对象

    const XTFInstrument*    getLeftInstrument() const;      ///< 左腿合约对象指针
    const XTFInstrument*    getRightInstrument() const;     ///< 右腿合约对象指针
    const XTFCombPosition*  getCombPosition(XTFCombHedgeFlag combHedgeFlag = XTF_COMB_HF_SpecSpec) const; ///< 查询组合持仓对象
    long                    getCombPriority(XTFCombHedgeFlag combHedgeFlag = XTF_COMB_HF_SpecSpec) const; ///< 查询组合优先级
    const XTFExchange*      getExchange() const;            ///< 合约所属交易所
};


class XTF_API_EXPORT XTFCombPositionDetail {
public:
    int                     volume;                         ///< 组合持仓数量
    double                  margin;                         ///< 占用保证金总额（左腿保证金+右腿保证金）
    double                  paidMargin;                     ///< 实付保证金总额（优惠后的实付保证金，左腿保证金或者右腿保证金）
    XTFTradeID              leftTradeID;                    ///< 左腿合约的成交编号
    XTFTradeID              rightTradeID;                   ///< 右腿腿合约的成交编号
    mutable XTFUserData     userData;                       ///< 保留给用户使用的数据对象
};


class XTF_API_EXPORT XTFCombPosition {
public:
    int                     position;                       ///< 组合仓位总数量
    double                  margin;                         ///< 占用保证金总额，所有组合明细的左腿保证金+右腿保证金之和
    double                  paidMargin;                     ///< 实付保证金总额，所有组合明细的实付保证金之和
    XTFCombHedgeFlag        combHedgeFlag;                  ///< 组合投机套保标志
    mutable XTFUserData     userData;                       ///< 保留给用户使用的数据对象
    const XTFCombInstrument* getCombInstrument() const;     ///< 关联的组合合约
    int                     getCombPositionDetailCount() const; ///< 组合明细数量
    const XTFCombPositionDetail& getCombPositionDetail(int pos) const; ///< 组合明细
};


class XTF_API_EXPORT XTFPositionCombEvent {
public:
    XTFSysOrderID           sysOrderID;                     ///< 柜台流水号
    XTFLocalOrderID         localOrderID;                   ///< 用户填写的本地编号
    XTFExchangeOrderID      exchangeOrderID;                ///< 交易所报单编号
    XTFCombHedgeFlag        combHedgeFlag;                  ///< 组合投保标志
    XTFCombAction           combAction;                     ///< 组合行为类型(组合/解锁)
    uint32_t                combVolume;                     ///< 数量
    XTFTime                 combTime;                       ///< 组合时间
    const XTFCombInstrument*combInstrument;                 ///< 组合合约
};


#pragma pack()

#endif
