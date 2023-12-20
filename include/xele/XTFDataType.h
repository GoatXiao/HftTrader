/**
 * @file        XTFDataType.h
 * @brief       基础数据类型头文件
 * @details     本文件定义基础数据类型
 * @author      xinweiz
 * @date        2022-06-22
 * @version     v4.1
 * @copyright   南京艾科朗克信息科技有限公司
*/


#ifndef XTF_DATA_TYPE_H
#define XTF_DATA_TYPE_H

#include <cstdint>

typedef char XTFExchangeID[12];                 ///< 交易所ID数据类型
typedef char XTFProductID[16];                  ///< 品种数据类型
typedef char XTFProductGroupID[16];             ///< 品种组数据类型
typedef char XTFInstrumentID[32];               ///< 合约ID数据类型
typedef char XTFAccountID[20];                  ///< 账号ID数据类型
typedef char XTFDate[9];                        ///< 时间数据类型
typedef char XTFTime[9];                        ///< 时间数据类型

typedef int64_t XTFExchangeOrderID;             ///< 交易所报单编号数据类型
typedef int32_t XTFSysOrderID;                  ///< 柜台流水号数据类型
typedef int32_t XTFLocalOrderID;                ///< 本地报单编号数据类型（用户定义）
typedef int32_t XTFLocalActionID;               ///< 本地撤单编号数据类型（用户定义）
typedef int64_t XTFTradeID;                     ///< 成交编号数据类型

///< 报单编号类型
const uint8_t XTF_OIDT_Local = 0;               ///< 用户本地编号
const uint8_t XTF_OIDT_System = 1;              ///< 柜台流水编号
const uint8_t XTF_OIDT_Exchange = 2;            ///< 市场交易所编号
typedef uint8_t XTFOrderIDType;

///< 品种类型
const uint8_t XTF_PC_Futures = 1;               ///< 期货类型
const uint8_t XTF_PC_Options = 2;               ///< 期货期权类型
const uint8_t XTF_PC_Combination = 3;           ///< 组合类型
const uint8_t XTF_PC_SpotOptions = 6;           ///< 现货期权类型
typedef uint8_t XTFProductClass;

///< 期权类型
const uint8_t XTF_OT_NotOptions = 0;            ///< 非期权
const uint8_t XTF_OT_CallOptions = 1;           ///< 看涨
const uint8_t XTF_OT_PutOptions = 2;            ///< 看跌
typedef uint8_t XTFOptionsType;

///< 买卖方向
const uint8_t XTF_D_Buy = 1;                    ///< 买
const uint8_t XTF_D_Sell = 2;                   ///< 卖
const uint8_t XTF_D_Invalid = 255;              ///< 无效值
typedef uint8_t XTFDirection;

///< 持仓多空方向
const uint8_t XTF_PD_Long = 1;                  ///< 多头
const uint8_t XTF_PD_Short = 2;                 ///< 空头
typedef uint8_t XTFPositionDirection;

///< 持仓日期（暂未使用）
const uint8_t XTF_PSD_Today = 1;                ///< 今仓
const uint8_t XTF_PSD_History = 2;              ///< 昨仓
typedef uint8_t XTFPositionDateType;

///< 投机套保标志，当前仅支持投机
const uint8_t XTF_HF_Invalid = 0;               ///< 无效
const uint8_t XTF_HF_Speculation = 1;           ///< 投机
const uint8_t XTF_HF_Arbitrage = 2;             ///< 套利
const uint8_t XTF_HF_Hedge = 3;                 ///< 保值
const uint8_t XTF_HF_MaxCount = 4;              ///< 保留内部使用
typedef uint8_t XTFHedgeFlag;

///< 出入金方向
const uint8_t XTF_CASH_In = 1;                  ///< 入金
const uint8_t XTF_CASH_Out = 2;                 ///< 出金
typedef uint8_t XTFCashDirection;

///< 开平标志
const uint8_t XTF_OF_Open = 0;                  ///< 开仓
const uint8_t XTF_OF_Close = 1;                 ///< 平仓
const uint8_t XTF_OF_ForceClose = 2;            ///< 强平（暂不支持）
const uint8_t XTF_OF_CloseToday = 3;            ///< 平今
const uint8_t XTF_OF_CloseYesterday = 4;        ///< 平昨
const uint8_t XTF_OF_Invalid = 255;             ///< 无效值
typedef uint8_t XTFOffsetFlag;

///< 报单标志
const uint8_t XTF_ODF_Normal                    = 0;    ///< 普通报单
const uint8_t XTF_ODF_CombinePosition           = 1;    ///< 组合持仓（暂不支持）
const uint8_t XTF_ODF_OptionsExecute            = 2;    ///< 行权报单
const uint8_t XTF_ODF_OptionsSelfClose          = 3;    ///< 对冲报单
const uint8_t XTF_ODF_Warm                      = 254;  ///< 预热报单（暂不支持）
const uint8_t XTF_ODF_Invalid                   = 255;  ///< 无效值
typedef uint8_t XTFOrderFlag;

///< 报单类型 XTF_ODF_Normal
const uint8_t XTF_ODT_Limit             = 0;    ///< GFD报单（限价）
const uint8_t XTF_ODT_FAK               = 1;    ///< FAK报单（限价）
const uint8_t XTF_ODT_FOK               = 2;    ///< FOK报单（限价）
const uint8_t XTF_ODT_Market            = 3;    ///< 市价报单（暂不支持）
///< 报单类型 XTF_ODF_OptionsExecute
const uint8_t XTF_ODT_SelfClose         = 21;   ///< 期权对冲
const uint8_t XTF_ODT_NotSelfClose      = 22;   ///< 不对冲
///< 报单类型 XTF_ODF_OptionsSelfClose
const uint8_t XTF_ODT_SelfCloseOptions  = 31;   ///< 期权对冲
const uint8_t XTF_ODT_SelfCloseFutures  = 32;   ///< 履约对冲（期权期货对冲卖方履约后的期货仓位）
const uint8_t XTF_ODT_Invalid           = 255;  ///< 无效值
typedef uint8_t XTFOrderType;

///< 报单状态
const uint8_t XTF_OS_Created = 0;               ///< 报单已创建，为报单的初始状态。
                                                ///< 报单发送成功后，API会立即创建报单对象并设置为初始状态，此时报单的柜台流水号、
                                                ///< 交易所编号等字段都是无效的。当收到柜台返回的应答和回报时，会切换为其它状态，
                                                ///< 用户可以根据此状态查询哪些报单还没有收到应答和回报。
const uint8_t XTF_OS_Accepted = 1;              ///< 报单已通过柜台校验并送往交易所，且已收到交易所的报单录入结果（报单应答消息）。
                                                ///< 此状态为临时状态，在报单应答和报单回报乱序的场景下，会跳过此状态，直接进入后续状态。
                                                ///< 后续状态包括：XTF_OS_Queuing | XTF_OS_Canceled | XTF_OS_PartTraded | XTF_OS_AllTraded | XTF_OS_Rejected
const uint8_t XTF_OS_Queuing = 2;               ///< 报单未成交，进入队列等待撮合，IOC订单无此状态
const uint8_t XTF_OS_Canceled = 3;              ///< 报单已被撤销，可能是客户主动撤销，也可能是IOC报单被交易所系统自动撤销
const uint8_t XTF_OS_PartTraded = 4;            ///< 部分成交
const uint8_t XTF_OS_AllTraded = 5;             ///< 完全成交
const uint8_t XTF_OS_Rejected = 6;              ///< 报单被柜台或交易所拒绝，错误码通过onOrder事件的errorCode参数返回，根据API手册或者官方文档平台错误码说明得到具体原因
const uint8_t XTF_OS_Received = 7;              ///< 报单发送到柜台，柜台已接收此报单，并通过柜台风控，下一时刻将发往交易所。
                                                ///< 此状态的报单，柜台流水号是有效的。可以根据此流水号进行撤单操作。
const uint8_t XTF_OS_Invalid = 255;             ///< 无效值
typedef uint8_t XTFOrderStatus;

///< 席位选择
const uint8_t XTF_CS_Auto = 0;                  ///< 不指定交易所席位链接
const uint8_t XTF_CS_Fixed = 1;                 ///< 指定交易所席位链接
const uint8_t XTF_CS_Unknown = 9;               ///< 历史报单回报，无法确认报单通道选择类型
typedef uint8_t XTFChannelSelectionType;

///< 保证金计算价格类型(Margin Price Type)
const uint8_t XTF_MPT_MaxLastSettlementOrLastPrice = 0; ///< 最新价和昨结算价之间的较大值（期权空头暂不支持）
const uint8_t XTF_MPT_LastSettlementPrice = 1;          ///< 昨结算价计算保证金（期权空头默认）
const uint8_t XTF_MPT_OrderPrice = 2;                   ///< 报单价计算保证金（期权空头有效）
const uint8_t XTF_MPT_OpenPrice = 3;                    ///< 开仓价计算保证金（期货默认）
const uint8_t XTF_MPT_LastPrice = 4;                    ///< （暂未使用）最新价
const uint8_t XTF_MPT_AverageOpenPrice = 5;             ///< （暂未使用）开仓均价
const uint8_t XTF_MPT_AveragePrice = 6;                 ///< （暂未使用）市场平均成交价
const uint8_t XTF_MPT_LimitPrice = 7;                   ///< （暂未使用）涨跌停价
typedef uint8_t XTFMarginPriceType;

///< 合约状态
const uint8_t XTF_IS_BeforeTrading = 0;                 ///< 开盘前
const uint8_t XTF_IS_NoTrading = 1;                     ///< 非交易
const uint8_t XTF_IS_Continuous = 2;                    ///< 连续交易
const uint8_t XTF_IS_AuctionOrdering = 3;               ///< 集合竞价报单
const uint8_t XTF_IS_AuctionBalance = 4;                ///< 集合竞价价格平衡
const uint8_t XTF_IS_AuctionMatch = 5;                  ///< 集合竞价撮合
const uint8_t XTF_IS_Closed = 6;                        ///< 收盘
const uint8_t XTF_IS_TransactionProcessing = 7;         ///< 交易业务处理
const uint8_t XTF_IS_TransactionMatchPause = 8;         ///< 集合竞价暂停
const uint8_t XTF_IS_TransactionTradePause = 9;         ///< 连续交易暂停
const uint8_t XTF_IS_Alarm = 10;                        ///< 自动转换报警
const uint8_t XTF_IS_Invalid = 255;                     ///< 无效值
typedef uint8_t XTFInstrumentStatus;

///< 报单操作类型
const uint8_t XTF_OA_Insert     = 1;                    ///< 插入报单
const uint8_t XTF_OA_Cancel     = 2;                    ///< 撤销报单
const uint8_t XTF_OA_Suspend    = 3;                    ///< 挂起报单（暂未使用）
const uint8_t XTF_OA_Resume     = 4;                    ///< 恢复报单（暂未使用）
const uint8_t XTF_OA_Update     = 5;                    ///< 更新报单（暂未使用）
const uint8_t XTF_OA_Return     = 9;                    ///< 报单回报
const uint8_t XTF_OA_Invalid    = 255;                  ///< 无效的报单操作
typedef uint8_t XTFOrderActionType;

///< API事件（已废弃，改成 onServerReboot() 接口通知用户）
const uint8_t XTF_AE_ServerRestarted = 1;               ///< 发现服务器重启，客户端退出前推送的事件

///< 事件通知类型
const int XTF_EVENT_TYPE_Public   = 1;
const int XTF_EVENT_TYPE_Private  = 2;
typedef int XTFEventType;

///< 事件通知ID-API定义的事件ID
const int XTF_EVT_AccountCashInOut              = 0x1001;       ///< 账户出入金发生变化通知
const int XTF_EVT_ExchangeChannelConnected      = 0x1011;       ///< 交易所交易通道已连接通知
const int XTF_EVT_ExchangeChannelDisconnected   = 0x1012;       ///< 交易所交易通道已断开通知
const int XTF_EVT_InstrumentStatusChanged       = 0x1021;       ///< 合约状态发生变化通知

///< 事件通知ID-柜台返回的事件ID
const int XTF_EVT_PUB_StopBusiness              = 1;            ///< 柜台业务停止通知，api将无法登录
const int XTF_EVT_PRI_InvestorPrc               = 3;            ///< 投资者风控变化
const int XTF_EVT_PRI_InstrumentPrc             = 4;            ///< 合约风控变化

typedef int XTFEventID;

/////////////////////////////////////////////////////////////////////////
/// 风控ID类型
const int XTF_PRC_InvestorTradeEnabled          = 1;            ///< 投资者风控 允许交易
typedef int XTFPrcID;

/////////////////////////////////////////////////////////////////////////
/// 组合类型
const uint8_t XTF_COMB_SPL                      = 0;            ///< 期货对锁
const uint8_t XTF_COMB_OPL                      = 1;            ///< 期权对锁
const uint8_t XTF_COMB_SP                       = 2;            ///< 跨期套利
const uint8_t XTF_COMB_SPC                      = 3;            ///< 跨品种套利
const uint8_t XTF_COMB_BLS                      = 4;            ///< 买入垂直价差
const uint8_t XTF_COMB_BES                      = 5;            ///< 卖出垂直价差
const uint8_t XTF_COMB_CAS                      = 6;            ///< 期权日历价差
const uint8_t XTF_COMB_STD                      = 7;            ///< 期权跨式
const uint8_t XTF_COMB_STG                      = 8;            ///< 期权宽跨式
const uint8_t XTF_COMB_BFO                      = 9;            ///< 买入期货期权
const uint8_t XTF_COMB_SFO                      = 10;           ///< 卖出期货期权
const uint8_t XTF_COMB_Invalid                  = 255;
typedef uint8_t XTFCombType;

/////////////////////////////////////////////////////////////////////////
/// 组合方向类型
const uint8_t XTF_COMB_D_LongShort              = 0;
const uint8_t XTF_COMB_D_ShortLong              = 1;
const uint8_t XTF_COMB_D_Invalid                = 255;
typedef uint8_t XTFCombDirection;

/// 组合投机套保标志，当前仅支持投机-投机
const uint8_t XTF_COMB_HF_Invalid               = 0;            ///< 无效
const uint8_t XTF_COMB_HF_SpecSpec              = 1;            ///< 投机-投机
const uint8_t XTF_COMB_HF_SpecHedge             = 2;            ///< 投机-保值
const uint8_t XTF_COMB_HF_HedgeHedge            = 3;            ///< 保值-保值
const uint8_t XTF_COMB_HF_HedgeSpec             = 4;            ///< 保值-投机
const uint8_t XTF_COMB_HF_MaxCount              = 5;            ///< 保留内部使用
typedef uint8_t XTFCombHedgeFlag;

/// 组合动作类型
const uint8_t XTF_COMB_AT_Combine               = 1;            ///< 组合
const uint8_t XTF_COMB_AT_Uncombine             = 2;            ///< 拆组合
const uint8_t XTF_COMB_AT_Invalid               = 255;
typedef uint8_t XTFCombAction;

/////////////////////////////////////////////////////////////////////////
/// 行权对冲

/// 期权行权/对冲执行结果
const char XTF_OER_Error                        = 'e';          ///< 执行失败
const char XTF_OER_NoExec                       = 'n';          ///< 没有执行
const char XTF_OER_Canceled                     = 'c';          ///< 已经取消
const char XTF_OER_Success                      = '0';          ///< 执行成功
const char XTF_OER_NoPosition                   = '1';          ///< 期权持仓不够
const char XTF_OER_NoDeposit                    = '2';          ///< 资金不够
const char XTF_OER_NoParticipant                = '3';          ///< 会员不存在
const char XTF_OER_NoClient                     = '4';          ///< 客户不存在
const char XTF_OER_NoInstrument                 = '6';          ///< 合约不存在
const char XTF_OER_NoRight                      = '7';          ///< 没有执行权限
const char XTF_OER_InvalidVolume                = '8';          ///< 不合理的数量
const char XTF_OER_NoEnoughHistoryTrade         = '9';          ///< 没有足够的历史成交
typedef char XTFOptionsExecResult;

#endif
