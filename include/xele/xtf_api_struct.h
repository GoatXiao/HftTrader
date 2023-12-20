/**
 * @file       xtf_api_struct.h
 * @brief      柜台和客户端通信的报文结构体
 * @details    
 1. 结构体名称和字段名称大小完全兼容3代结构体; 
 2. 部分结构体新增字段; 
 3. 新增部分结构体;
 4. 所有结构体都是按照32字节或64字节对齐;
 * @author     accelecom
 * @date       2022-07-20
 * @version    V4.1
 * @copyright  accelecom
 */

#ifndef __XTF_API_STRUCT_H__
#define __XTF_API_STRUCT_H__
#include <cassert>
#include <xele/xtf_api_datatype.h>

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(push,1)
///*************************以下是FAIR协议消息头和域结构体*************************///
/// 结构体说明:      FAIR格式消息头结构体(消息头);
/// 对齐说明      : 字段1字节对齐，总共14字节;
/// 变动点       : 是指和3代协议结构体的变动点;
/// 1.  Token字段类型由2字节扩展为4字节;
/// 2.  结构体总共为14字节;
struct CXeleFairHeader {
  ///消息ID
  TXeleFtdcMsgIdType        MessageId;
  ///客户序号
  TXeleFtdcClientIndexType  ClientIndex;
  ///客户令牌
  TXeleFtdcClientTokenType  Token;
  ///消息序列号
  TXeleFtdcSequenceNoType   SeqNo;
  ///请求序号,建议单调递增, 有效范围为(0~0xfffffffe)
  TXeleFtdcReqIdType        RequestID;
};
static_assert(sizeof(CXeleFairHeader) == 14, "CXeleFairHeader size error");


/// 结构体说明:      FAIR格式 报单输入消息 域结构体(消息域)
/// 对齐说明      : 字段1字节对齐，总共50字节;
/// 变动点       : 是指和3代协议结构体的变动点;
/// 1. 新增InstrumentIndex字段, 4字节, 报单时需要填写;
/// 2. 结构体总共50字节;
struct CXeleFairInputOrderFieldRaw{
  ///本地报单编号
  TXeleFtdcOrderLocalNoType          OrderLocalNo;
  ///报单价格
  TXeleFtdcPriceType                 LimitPrice;  
  ///合约代码
  TXeleFtdcInstruIDType              InstrumentID;
  ///数量
  TXeleFtdcVolumeTotalOriginalType   VolumeTotalOriginal;
  ///输入报单类型
  TXeleFtdcInsertType                InsertType;
  ///最小成交数量
  TXeleFtdcMinVolumeType             MinVolume;
  ///指定前置信息(不指定前置填写为0， 指定前置需要加上偏移量10, 如指定前置3，则需要填写13)
  TXeleFtdcExchangeFrontEnumType     ExchangeFront;
  
  ///----以下为新增字段--------
  ///合约序号
  TXeleFtdcInstrumentIndexType       InstrumentIndex;
  ///预留字段
  char                               Rsv[12];
};
static_assert(sizeof(CXeleFairInputOrderFieldRaw) == 50, "CXeleFairInputOrderFieldRaw size error");


/// 结构体说明:      FAIR格式 报单操作消息域结构体(消息域)
/// 对齐说明      : 字段1字节对齐，总共50字节;
/// 变动点       : 是指和3代协议结构体的变动点;
/// 1. 新增OrderLocalNo字段, 4字节，当前版本未使用;
/// 2. 结构体总共50字节;
struct CXeleFairOrderActionFieldRaw {
  ///本地报单操作编号
  TXeleFtdcActionLocalNoType ActionLocalNo;
  ///系统报单编号
  TXeleFtdcOrderSysNoType    OrderSysNo; 
  ///报单操作标志
  TXeleFtdcActionFlagType    ActionFlag;
  
  ///----以下为新增字段--------
  ///本地报单编号
  TXeleFtdcOrderLocalNoType  OrderLocalNo;  
  ///预留字段
  char Rsv[37];
};
static_assert(sizeof(CXeleFairOrderActionFieldRaw) == 50, "CXeleFairOrderActionFieldRaw size error");


///*************************以下是FAIR协议报撤单请求完整消息结构体*************************///
/// 结构体说明:      FAIR格式 报单输入消息完整结构体(包括头和域)
/// 对齐说明      : 字段1字节对齐，结构体64字节对齐，总共64字节;
/// 变动点       : 是指和3代协议结构体的变动点;
/// 1. Token字段由2字节扩展为4字节;
/// 2. 新增InstrumentIndex字段, 4字节, 报单时需要填写;
/// 3. 结构体64字节对齐，总共50字节;
struct CXeleFairInputOrderMsg {
  ///------FAIR消息头------
  ///消息ID
  TXeleFtdcMsgIdType        MessageId;
  ///客户序号
  TXeleFtdcClientIndexType  ClientIndex;
  ///客户令牌
  TXeleFtdcClientTokenType  Token;
  ///消息序列号
  TXeleFtdcSequenceNoType   SeqNo;
  ///请求序号,建议单调递增, 有效范围为(0~0xfffffffe)
  TXeleFtdcReqIdType        RequestID;

  ///------FAIR消息域------
  ///本地报单编号
  TXeleFtdcOrderLocalNoType OrderLocalNo;
  ///报单价格
  TXeleFtdcPriceType        LimitPrice;  
  ///合约代码
  TXeleFtdcInstruIDType     InstrumentID;
  ///数量
  TXeleFtdcVolumeTotalOriginalType   VolumeTotalOriginal;
  ///输入报单类型(具体值类型请 参考TXeleFtdcInsertType 定义说明)
  TXeleFtdcInsertType                InsertType;
  ///最小成交数量
  TXeleFtdcMinVolumeType             MinVolume;
  ///指定前置信息(不指定前置填写为0， 指定前置需要加上偏移量10, 如指定前置3，则需要填写13)
  TXeleFtdcExchangeFrontEnumType     ExchangeFront;
  
  ///------以下为新增字段------
  ///合约序号
  TXeleFtdcInstrumentIndexType       InstrumentIndex;
  ///预留字段
  char                               Rsv[12];
};
static_assert(sizeof(CXeleFairInputOrderMsg) == 64, "CXeleFairInputOrderMsg size error");

/// 结构体说明:      FAIR格式 报单操作消息完整结构体(包括头和域)
/// 对齐说明      : 字段1字节对齐，结构体64字节对齐， 总共64字节;
/// 变动点       : 是指和3代协议结构体的变动点;
/// 1. Token字段由2字节扩展为4字节;
/// 2. 新增OrderLocalNo字段, 4字节，当前版本未使用;
/// 3. 结构体64字节对齐，总共64字节;
struct CXeleFairOrderActionMsg{
  ///------FAIR消息头------
  ///消息ID
  TXeleFtdcMsgIdType        MessageId;
  ///客户序号
  TXeleFtdcClientIndexType  ClientIndex;
  ///客户令牌
  TXeleFtdcClientTokenType  Token;
  ///消息序列号
  TXeleFtdcSequenceNoType   SeqNo;
  ///请求序号,建议单调递增, 有效范围为(0~0xfffffffe)
  TXeleFtdcReqIdType        RequestID;
  
  ///------FAIR消息域------
  ///本地报单操作编号
  TXeleFtdcActionLocalNoType  ActionLocalNo;
  ///系统报单编号
  TXeleFtdcOrderSysNoType     OrderSysNo; 
  ///报单操作标志
  TXeleFtdcActionFlagType     ActionFlag;
  
  ///------以下为新增字段------
  ///本地报单编号
  TXeleFtdcOrderLocalNoType   OrderLocalNo;    
  ///预留字段
  char Rsv[37];
};
static_assert(sizeof(CXeleFairOrderActionMsg) == 64, "CXeleFairOrderActionMsg size error");

///*************************以下是FAIR协议组合请求完整消息结构体*************************///
/// 结构体说明:      FAIR格式 组合报单消息完整结构体(包括头和域)
/// 对齐说明      : 字段1字节对齐，结构体64字节对齐，总共64字节;
struct CXeleFairCombOrderMsg {
  ///------FAIR消息头------
  ///消息ID (0x69)
  TXeleFtdcMsgIdType        MessageId;
  ///客户序号
  TXeleFtdcClientIndexType  ClientIndex;
  ///客户令牌
  TXeleFtdcClientTokenType  Token;
  ///消息序列号
  TXeleFtdcSequenceNoType   SeqNo;
  ///请求序号,建议单调递增, 有效范围为(0~0xfffffffe)
  TXeleFtdcReqIdType        RequestID;

  ///------FAIR消息域------
  ///本地报单编号
  TXeleFtdcOrderLocalNoType OrderLocalNo;
  ///组合合约代码
  TXeleFtdcCombInstruIDType CombInstrumentID;
  ///组合投保标志
  TXeleFtdcCombHedgeType    CombHedgeFlag;
  ///组合状态类型(组合/解锁)
  TXeleFtdcCombActionType   CombAction; 
  ///组合数量
  TXeleFtdcCombVolumeType   Volume; 
  ///预留字段
  char                      Rsv[4];
};
static_assert(sizeof(CXeleFairCombOrderMsg) == 64, "CXeleFairCombOrderMsg size error");


///*************************以下是其他相关请求消息域结构体*************************///
/// 结构体说明: 客户登录请求域结构体
/// 对齐说明      : 字段1字节对齐，结构体32字节对齐, 总共224字节;
/// 变动点       : 是指和3代协议结构体的变动点;
/// 1. 新增部分字段, 新增的字段都是由接口赋值，用户无需填写;
/// 2. 结构体32字节对齐,总共224字节;
struct CXeleFtdcReqUserLoginField {
  ///未用字段
  char Unused1[7];
  /// 会话编号
  TXeleFtdcSessionType                 Session;
  ///交易用户代码
  TXeleFtdcAccountIDType               AccountID;
  ///未用字段
  char Unused2[14];
  ///密码
  TXeleFtdcPasswordType                Password;
  ///用户端产品信息 (程序自动填写)
  TXeleFtdcProductInfoType             UserProductInfo;
  ///接口端产品信息 (程序自动填写)
  TXeleFtdcProductInfoType             InterfaceProductInfo;
  ///组合合约授权标记(0:不授权，1:授权, 只有大商所使用)
  TXeleFtdcCombinedContractAuthorizeType CombinedContractAuthorize;
  ///产品保留字段(程序自动填写)
  TXeleFtdcLoginReserve1Type  ProductReserve1;
  ///产品保留字段(程序自动填写)
  TXeleFtdcLoginReserve1Type  ProductReserve2;
  ///产品保留字段(程序自动填写)
  TXeleFtdcLoginReserve1Type  ProductReserve3;
  ///产品保留字段(程序自动填写)
  TXeleFtdcLoginReserve1Type  ProductReserve4;

  ///----以下为新增字段--------
  ///报单的IP地址
  TXeleFtdcOrderIpType         OrderIp;
  ///报单的port号
  TXeleFtdcOrderPortType       OrderPort;
  ///报单的mac地址
  TXeleFtdcOrderMacType        OrderMac;
  ///协议版本号
  TXeleFtdcProrocalVersionType ProtocolVersion;
  ///客户端用户类型
  TXeleFtdcApiUserType         UserType;
  ///API模式
  TXeleFtdcApiModeType         ApiMode;
  ///校验和信息
  TXeleFtdcCheckSumType        CheckSum;
  ///无效的原因
  TXeleFtdcInvalidReasonType   InvalidReason;
  /// 预留字段
  char Rsv[18];
};
static_assert(sizeof(CXeleFtdcReqUserLoginField) == 224, "CXeleFtdcReqUserLoginField size error");


/// 结构体说明: 客户登出请求域结构体
/// 对齐说明      : 字段1字节对齐，结构体32字节对齐, 总共32字节;
/// 变动点       : 是指和3代协议结构体的变动点;
/// 1. 结构体32字节对齐, 总共32字节;
struct CXeleFtdcReqUserLogoutField {
  ///交易用户代码
  TXeleFtdcAccountIDType AccountID;
  ///未用字段
  char Unused[3];
  ///会员代码(未使用)
  TXeleFtdcParticipantIDType ParticipantID;
  ///预留字段
  char Rsv[30];
};
static_assert(sizeof(CXeleFtdcReqUserLogoutField) == 64, "CXeleFtdcReqUserLogoutField size error");


/// 结构体说明: 用户口令修改请求/响应域结构体
/// 对齐说明      : 字段1字节对齐，结构体64字节对齐, 总共128字节;
/// 变动点       : 是指和3代协议结构体的变动点;
/// 1. 结构体64字节对齐, 总共128字节;
struct CXeleFtdcUserPasswordUpdateField {
  ///交易用户代码
  TXeleFtdcAccountIDType     AccountID;
  ///未用字段
  char Unused[3];
  ///会员代码(未使用)
  TXeleFtdcParticipantIDType ParticipantID;   
  ///旧密码
  TXeleFtdcPasswordType  OldPassword;
  ///新密码
  TXeleFtdcPasswordType  NewPassword;
  /// 预留字段
  char Rsv[12];
};
static_assert(sizeof(CXeleFtdcUserPasswordUpdateField) == 128, "CXeleFtdcUserPasswordUpdateField size error");


/// 结构体说明: 用户资金查询请求域结构体
/// 对齐说明      : 字段1字节对齐，结构体32字节对齐, 总共32字节;
/// 变动点       : 是指和3代协议结构体的变动点;
/// 1. 结构体32字节对齐, 总共32字节;
struct CXeleFtdcQryClientAccountField {
  ///资金帐号
  TXeleFtdcAccountIDType AccountID;
  ///预留字段
  char Rsv[12];
};
static_assert(sizeof(CXeleFtdcQryClientAccountField) == 32, "CXeleFtdcQryClientAccountField size error");

/// 结构体说明: 用户持仓查询请求域结构体
/// 对齐说明      : 字段1字节对齐，结构体32字节对齐, 总共96字节;
/// 1. 当前仅支持组合持仓查询
struct CXeleFtdcQryClientPositionField {
  ///资金帐号
  TXeleFtdcAccountIDType        AccountID;
  ///品种信息
  TXeleFtdcProductIDType        ProductID;  
  ///合约代码
  TXeleFtdcCombInstruIDType     CombInstrumentID;
  ///交易所描述符
  TXeleFtdcExchangeDescriptorType ExchangeDescriptor;
  ///查询的持仓类型
  TXeleFtdcQryPositionType PositionType;
  ///预留字段
  char Rsv[29];
};
static_assert(sizeof(CXeleFtdcQryClientPositionField) == 96, "CXeleFtdcQryClientPositionField size error");

///*************************以下是响应/回报域结构体*************************///
/// 结构体说明: 用户登录响应域结构体
/// 对齐说明      : 字段1字节对齐，结构体32字节对齐, 总共192字节;
/// 变动点       : 是指和3代协议结构体的变动点;
/// 1. 新增部字段， 由柜台赋值;
/// 2. 结构体32字节对齐, 总共192字节;
struct CXeleFtdcRspUserLoginField {
  ///交易日
  TXeleFtdcDateType TradingDay;
  ///登录成功时间
  TXeleFtdcTimeType LoginTime;
  ///最大本地报单号
  TXeleFtdcOrderLocalIDType MaxOrderLocalID;
  ///交易用户代码
  TXeleFtdcAccountIDType AccountID;
  ///当前登录的交易所数目;
  TXeleFtdcExchangeNumType ExchangeNum;  
  ///当前登录交易所的交易用户编号;
  TXeleFtdcClientIndexType ClientIndex[XELE_EXCHANGE_LOGIN_NUM];
  ///当前登录交易所的交易用户令牌;
  TXeleFtdcClientTokenType Token[XELE_EXCHANGE_LOGIN_NUM]; 
  ///会员代码
  TXeleFtdcParticipantIDType ParticipantID;
  ///交易系统名称
  TXeleFtdcTradingSystemNameType TradingSystemName;
  ///数据中心代码(未使用)
  TXeleFtdcDataCenterIDType DataCenterID;
  ///会员私有流当前长度(未使用)
  TXeleFtdcSequenceNoType PrivateFlowSize;
  ///交易员私有流当前长度
  TXeleFtdcSequenceNoType UserFlowSize;

  ///------以下为新增字段------
  ///当前登录的交易所ID(表明对应的ClientIndex和token归属的交易所ID)
  TXeleFtdcExchangeDescriptorType ExchangeID[XELE_EXCHANGE_LOGIN_NUM];
  ///柜台版本号
  TXeleFtdcSystemVersionType      SystemVersion;
  ///柜台协议版本号
  TXeleFtdcProrocalVersionType    ProtocolVersion;
  ///柜台的唯一识别码
  TXeleFtdcSystemUUIDType         SystemUUID;
  /// 最后一次用户请求的报单编号
  TXeleFtdcOrderLocalNoType       LastLocalOrderNo;
  /// 最后一次用户请求的撤单编号
  TXeleFtdcOrderLocalNoType       LastActionLocalNo;;
  /// 会话编号
  TXeleFtdcSessionType Session;
  ///预留字段
  char Rsv[30];
};
static_assert(sizeof(CXeleFtdcRspUserLoginField) == 224, "CXeleFtdcRspUserLoginField size error");


/// 结构体说明: 用户登出响应域结构体
/// 对齐说明      : 字段1字节对齐，结构体32字节对齐, 总共32字节;
/// 变动点       : 是指和3代协议结构体的变动点;
/// 1. 结构体32字节对齐, 总共32字节;
struct CXeleFtdcRspUserLogoutField {
  ///交易用户代码
  TXeleFtdcAccountIDType AccountID;
  ///未用字段
  char Unused[3];
  ///会员代码(未使用)
  TXeleFtdcParticipantIDType ParticipantID; 
  /// 预留字段
  char Rsv[30];
};
static_assert(sizeof(CXeleFtdcRspUserLogoutField) == 64, "CXeleFtdcRspUserLogoutField size error");


/// 结构体说明: 用户资金查询响应域结构体
/// 对齐说明      : 字段1字节对齐，结构体32字节对齐, 总共160字节;
/// 变动点       : 是指和3代协议结构体的变动点;
/// 1. 结构体32字节对齐，总共160字节;
struct CXeleFtdcRspClientAccountField {
  ///交易日
  TXeleFtdcDateType TradingDay;
  ///结算组代码(未使用)
  TXeleFtdcSettlementGroupIDType SettlementGroupID;
  ///结算编号(未使用)
  TXeleFtdcSettlementIDType SettlementID;
  ///上次结算准备金
  TXeleFtdcMoneyType PreBalance;
  ///当前保证金总额
  TXeleFtdcMoneyType CurrMargin;
  ///平仓盈亏
  TXeleFtdcMoneyType CloseProfit;
  ///期权权利金收支
  TXeleFtdcMoneyType Premium;
  ///入金金额
  TXeleFtdcMoneyType Deposit;
  ///出金金额
  TXeleFtdcMoneyType Withdraw;
  ///期货结算准备金
  TXeleFtdcMoneyType Balance;
  ///可提资金
  TXeleFtdcMoneyType Available;
  ///资金帐号
  TXeleFtdcAccountIDType AccountID;
  ///冻结的保证金
  TXeleFtdcMoneyType FrozenMargin;
  ///冻结的权利金
  TXeleFtdcMoneyType FrozenPremium;
  ///基本准备金(未使用)
  TXeleFtdcMoneyType BaseReserve;
  ///浮动盈亏
  TXeleFtdcMoneyType FloatProfitAndLoss;
  /// 预留字段
  char Rsv[22];
};
static_assert(sizeof(CXeleFtdcRspClientAccountField) == 160, "CXeleFtdcRspClientAccountField size error");

/// 结构体说明: 用户组合持仓查询响应域结构体
/// 对齐说明      : 字段1字节对齐，结构体32字节对齐, 总共128字节;
struct CXeleFtdcRspClientPositionField {
  ///交易日
  TXeleFtdcDateType TradingDay;
  ///资金帐号
  TXeleFtdcAccountIDType AccountID;
  ///品种信息
  TXeleFtdcProductIDType    ProductID; 
  ///合约代码
  TXeleFtdcCombInstruIDType CombInstrumentID;
  ///投机套保标志
  TXeleFtdcHedgeFlagType HedgeFlag;
  ///单腿多头上日持仓
  TXeleFtdcVolumeType LongYdPosition;
  ///单腿多头今日持仓
  TXeleFtdcVolumeType LongPosition;
  ///单腿空头上日持仓
  TXeleFtdcVolumeType ShortYdPosition;
  ///单腿空头今日持仓
  TXeleFtdcVolumeType ShortPosition;
  ///单腿持仓成本
  TXeleFtdcMoneyType PositionCost;
  ///单腿昨日持仓成本
  TXeleFtdcMoneyType YdPositionCost;
  ///组合持仓
  TXeleFtdcVolumeType CombPosition;
  ///组合类型
  TXeleFtdcCombType   CombType;
  ///预留字段
  char Rsv[16];
};
static_assert(sizeof(CXeleFtdcRspClientPositionField) == 128, "CXeleFtdcRspClientPositionField size error");

/// 结构体说明: 用户报单响应/错单回报 域结构体
/// 对齐说明      : 字段1字节对齐，结构体32字节对齐, 总共192字节;
/// 变动点       : 是指和3代协议结构体的变动点;
/// 1. 结构体32字节对齐，总共192字节;
struct CXeleFtdcInputOrderField {
  ///系统报单编号
  TXeleFtdcOrderSystemNoType OrderSystemNo;
  ///未用字段
  char Unused1[9];
  ///会员代码
  TXeleFtdcParticipantIDType ParticipantID;
  ///客户代码
  TXeleFtdcClientIDType ClientID;
  ///交易用户代码
  TXeleFtdcUserIDType UserID;
  ///合约代码
  TXeleFtdcInstrumentIDType InstrumentID;
  ///报单价格条件
  TXeleFtdcOrderPriceTypeType OrderPriceType;
  ///买卖方向
  TXeleFtdcDirectionType Direction;
  ///组合开平标志
  TXeleFtdcCombOffsetFlagType CombOffsetFlag;
  ///组合投机套保标志
  TXeleFtdcCombHedgeFlagType CombHedgeFlag;
  ///价格
  TXeleFtdcPriceType LimitPrice;
  ///数量
  TXeleFtdcVolumeType VolumeTotalOriginal;
  ///有效期类型
  TXeleFtdcTimeConditionType TimeCondition;
  ///GTD日期
  TXeleFtdcDateType GTDDate;
  ///成交量类型
  TXeleFtdcVolumeConditionType VolumeCondition;
  ///最小成交量
  TXeleFtdcVolumeType MinVolume;
  ///触发条件
  TXeleFtdcContingentConditionType ContingentCondition;
  ///止损价
  TXeleFtdcPriceType StopPrice;
  ///强平原因
  TXeleFtdcForceCloseReasonType ForceCloseReason;
  ///本地报单编号
  TXeleFtdcOrderLocalNoType OrderLocalNo;
  ///未用字段
  char Unused2[9];
  ///自动挂起标志
  TXeleFtdcBoolType IsAutoSuspend;
  ///交易所报单编号, RspOrderInsert时有意义
  TXeleFtdcExchangeOrderSysIDType ExchangeOrderSysID;
  ///该笔报单的前置信息(同步单为-1)
  TXeleFtdcExchangeFrontType ExchangeFront;
  ///预留字段
  char Rsv[24];
};
static_assert(sizeof(CXeleFtdcInputOrderField) == 192, "CXeleFtdcInputOrderField size error");


/// 结构体说明: 用户报单操作响应/报单操作错误回报 域结构体
/// 对齐说明      : 字段1字节对齐，结构体64字节对齐, 总共128字节;
/// 变动点       : 是指和3代协议结构体的变动点;
/// 1. 结构体64字节对齐，总共128字节;
struct CXeleFtdcOrderActionField {
  ///系统报单编号
  TXeleFtdcOrderSystemNoType OrderSystemNo;
  ///未用字段
  char Unused1[9];
  ///本地报单编号
  TXeleFtdcOrderLocalNoType  OrderLocalNo;
  ///未用字段
  char Unused2[9];
  ///报单操作标志
  TXeleFtdcActionFlagType    ActionFlag;
  ///会员代码
  TXeleFtdcParticipantIDType ParticipantID;
  ///客户代码
  TXeleFtdcClientIDType      ClientID;
  ///交易用户代码
  TXeleFtdcUserIDType        UserID;
  ///价格
  TXeleFtdcPriceType         LimitPrice;
  ///数量变化
  TXeleFtdcVolumeType        VolumeChange;
  ///系统报单操作编号
  TXeleFtdcOrderLocalIDType  ActionLocalID;
  ///未用字段
  char Unused3[9];
  ///本地报单操作编号
  TXeleFtdcActionLocalNoType ActionLocalNo;
  ///预留字段
  char Rsv[21];
};
static_assert(sizeof(CXeleFtdcOrderActionField) == 128, "CXeleFtdcOrderActionField size error");


/// 结构体说明: 用户报单回报域结构体
/// 对齐说明      : 字段1字节对齐，结构体32字节对齐, 总共288字节;
/// 变动点       : 是指和3代协议结构体的变动点;
/// 1. 结构体32字节对齐，总共288字节;
struct CXeleFtdcOrderField {
  ///交易日
  TXeleFtdcDateType TradingDay;
  ///结算组代码(未使用)
  TXeleFtdcSettlementGroupIDType SettlementGroupID;
  ///结算编号(未使用)
  TXeleFtdcSettlementIDType SettlementID;
  ///系统报单编号
  TXeleFtdcOrderSystemNoType OrderSystemNo;
  ///未用字段
  char Unused1[9];
  ///会员代码
  TXeleFtdcParticipantIDType ParticipantID;
  ///客户代码
  TXeleFtdcClientIDType ClientID;
  ///交易用户代码
  TXeleFtdcUserIDType UserID;
  ///合约代码
  TXeleFtdcInstrumentIDType InstrumentID;
  ///报单价格条件
  TXeleFtdcOrderPriceTypeType OrderPriceType;
  ///买卖方向
  TXeleFtdcDirectionType Direction;
  ///组合开平标志
  TXeleFtdcCombOffsetFlagType CombOffsetFlag;
  ///组合投机套保标志
  TXeleFtdcCombHedgeFlagType CombHedgeFlag;
  ///价格
  TXeleFtdcPriceType LimitPrice;
  ///数量
  TXeleFtdcVolumeType VolumeTotalOriginal;
  ///有效期类型
  TXeleFtdcTimeConditionType TimeCondition;
  ///GTD日期
  TXeleFtdcDateType GTDDate;
  ///成交量类型
  TXeleFtdcVolumeConditionType VolumeCondition;
  ///最小成交量
  TXeleFtdcVolumeType MinVolume;
  ///触发条件
  TXeleFtdcContingentConditionType ContingentCondition;
  ///止损价
  TXeleFtdcPriceType StopPrice;
  ///强平原因
  TXeleFtdcForceCloseReasonType ForceCloseReason;
  ///本地报单编号
  TXeleFtdcOrderLocalNoType OrderLocalNo;
  ///未用字段
  char Unused2[9];
  ///自动挂起标志
  TXeleFtdcBoolType IsAutoSuspend;
  ///报单来源(未使用)
  TXeleFtdcOrderSourceType OrderSource;
  ///报单状态
  TXeleFtdcOrderStatusType OrderStatus;
  ///报单类型(未使用)
  TXeleFtdcOrderTypeType OrderType;
  ///今成交数量(未使用)
  TXeleFtdcVolumeType VolumeTraded;
  ///剩余数量
  TXeleFtdcVolumeType VolumeTotal;
  ///报单日期
  TXeleFtdcDateType InsertDate;
  ///插入时间
  TXeleFtdcTimeType InsertTime;
  ///激活时间(未使用)
  TXeleFtdcTimeType ActiveTime;
  ///挂起时间(未使用)
  TXeleFtdcTimeType SuspendTime;
  ///最后修改时间(未使用)
  TXeleFtdcTimeType UpdateTime;
  ///撤销时间(未使用)
  TXeleFtdcTimeType CancelTime;
  ///最后修改交易用户代码
  TXeleFtdcUserIDType ActiveUserID;
  ///优先权(未使用)
  TXeleFtdcPriorityType Priority;
  ///按时间排队的序号(未使用)
  TXeleFtdcTimeSortIDType TimeSortID;
  ///交易所报单编号
  TXeleFtdcExchangeOrderSysIDType ExchangeOrderSysID;
  ///该笔报单的前置信息(同步单为-1)
  TXeleFtdcExchangeFrontType ExchangeFront;
  ///预留字段
  char Rsv[5];
};
static_assert(sizeof(CXeleFtdcOrderField) == 288, "CXeleFtdcOrderField size error");


/// 结构体说明: 用户报单成交回报域结构体
/// 对齐说明      : 字段1字节对齐，结构体32字节对齐, 总共192字节;
/// 变动点       : 是指和3代协议结构体的变动点;
/// 1. 结构体32字节对齐，总共192字节;
struct CXeleFtdcTradeField {
  ///交易日
  TXeleFtdcDateType TradingDay;
  ///结算组代码(未使用)
  TXeleFtdcSettlementGroupIDType SettlementGroupID;
  ///结算编号(未使用)
  TXeleFtdcSettlementIDType SettlementID;
  ///成交编号
  TXeleFtdcTradeIDType TradeID;
  ///买卖方向
  TXeleFtdcDirectionType Direction;
  ///系统报单编号
  TXeleFtdcOrderSystemNoType OrderSystemNo;
  ///未用字段
  char Unused1[9];
  ///会员代码
  TXeleFtdcParticipantIDType ParticipantID;
  ///客户代码
  TXeleFtdcClientIDType ClientID;
  ///交易角色(未使用)
  TXeleFtdcTradingRoleType TradingRole;
  ///资金帐号(未使用)
  TXeleFtdcAccountIDType AccountID;
  ///合约代码
  TXeleFtdcInstrumentIDType InstrumentID;
  ///开平标志
  TXeleFtdcOffsetFlagType OffsetFlag;
  ///投机套保标志
  TXeleFtdcHedgeFlagType HedgeFlag;
  ///价格
  TXeleFtdcPriceType Price;
  ///数量
  TXeleFtdcVolumeType Volume;
  ///成交时间
  TXeleFtdcTimeType TradeTime;
  ///成交类型(未使用)
  TXeleFtdcTradeTypeType TradeType;
  ///成交价来源(未使用)
  TXeleFtdcPriceSourceType PriceSource;
  ///交易用户代码
  TXeleFtdcUserIDType UserID;
  ///本地报单编号
  TXeleFtdcOrderLocalNoType OrderLocalNo;
  ///未用字段
  char Unused2[9];
  ///交易所报单编号
  TXeleFtdcExchangeOrderSysIDType ExchangeOrderSysID;
  ///该笔报单的前置信息(同步单为-1)
  TXeleFtdcExchangeFrontType ExchangeFront;
  ///预留字段
  char Rsv[27];
};
static_assert(sizeof(CXeleFtdcTradeField) == 224, "CXeleFtdcTradeField size error");

/// 结构体说明: 用户组合报单回报/响应
/// 对齐说明      : 字段1字节对齐，结构体32字节对齐, 总共128字节;
struct CXeleFtdcCombOrderField {
  ///客户代码
  TXeleFtdcClientIDType      ClientID;
  ///交易用户代码
  TXeleFtdcUserIDType        UserID;
  ///系统报单编号
  TXeleFtdcOrderLocalNoType  OrderSystemNo;
  ///本地报单编号
  TXeleFtdcOrderLocalNoType  OrderLocalNo;
  ///组合合约代码
  TXeleFtdcCombInstruIDType  CombInstrumentID;
  ///组合投保标志
  TXeleFtdcCombHedgeType     CombHedgeFlag;
  ///组合状态类型(组合/解锁)
  TXeleFtdcCombActionType    CombAction;
  ///数量
  TXeleFtdcCombVolumeType    Volume; 
  ///组合时间
  TXeleFtdcCombTimeType      CombTime; 
  ///交易所报单编号
  TXeleFtdcExchangeOrderSysIDType ExchangeOrderSysID;
  ///预留字段
  char Rsv[21];
};
static_assert(sizeof(CXeleFtdcCombOrderField) == 128, "CXeleFtdcCombOrderField size error");


/// 结构体说明: 合约状态域结构体
/// 对齐说明      : 字段1字节对齐，结构体32字节对齐, 总共96字节;
/// 变动点       : 是指和3代协议结构体的变动点;
/// 1. 结构体32字节对齐，总共96字节;
/// 2. 新增ExchangeID字段
struct CXeleFtdcInstrumentStatusField {
  ///结算组代码(未使用)
  TXeleFtdcSettlementGroupIDType SettlementGroupID;
  ///合约代码
  TXeleFtdcInstrumentIDType InstrumentID;
  ///合约交易状态
  TXeleFtdcInstrumentStatusType InstrumentStatus;
  ///交易阶段编号
  TXeleFtdcTradingSegmentSNType TradingSegmentSN;
  ///进入本状态时间
  TXeleFtdcTimeType EnterTime;
  ///进入本状态原因
  TXeleFtdcInstStatusEnterReasonType EnterReason;
  ///进入本状态日期
  TXeleFtdcDateType	EnterDate;
  /// 交易所ID
  TXeleFtdcExchangeDescriptorType   ExchangeID;
  ///品种 9字节
  TXeleFtdcProductIDType ProductID;
  ///品种组 9字节
  TXeleFtdcProductIDType ProductGroupID;
  char Rsv[13];
};
static_assert(sizeof(CXeleFtdcInstrumentStatusField) == 96, "CXeleFtdcInstrumentStatusField size error");


/// 结构体说明: 响应信息域结构体
/// 对齐说明      : 字段1字节对齐，结构体32字节对齐, 总共96字节;
/// 变动点       : 是指和3代协议结构体的变动点;
/// 1. 结构体32字节对齐，总共96字节;
struct CXeleFtdcRspInfoField {
  ///错误代码
  TXeleFtdcErrorIDType ErrorID;
  ///错误信息
  TXeleFtdcErrorMsgType ErrorMsg;
  ///预留字段
  char Rsv[11];
};
static_assert(sizeof(CXeleFtdcRspInfoField) == 96, "CXeleFtdcRspInfoField size error");


/// 结构体说明: 订阅请求/响应 域结构体
/// 对齐说明      : 字段1字节对齐，结构体32字节对齐, 总共32字节;
/// 变动点       : 是指和3代协议结构体的变动点;
/// 1. 结构体32字节对齐，总共32字节;
struct CXeleFtdcDisseminationField {
  ///订阅序列系列类别号
  TXeleFtdcSequenceSeriesType SequenceSeries;
  ///订阅序列号
  TXeleFtdcSequenceNoType SequenceNo;
  /// 预留字段
  char Rsv[26];
};
static_assert(sizeof(CXeleFtdcDisseminationField) == 32, "CXeleFtdcDisseminationField size error");


/// 结构体说明: 交易所前置响应 域结构体
/// 对齐说明      : 字段1字节对齐，结构体32字节对齐, 总共288字节;
/// 变动点       : 是指和3代协议结构体的变动点;
/// 1. 结构体32字节对齐，总共288字节;
struct CXeleFtdcRspExchangeFrontField {
  ///交易所标志
  TXeleFtdcExchangNoType   ExchangeID;
  ///交易前置数量
  TXeleFtdcFrontCountType  FrontCount;
  ///交易前置代码列表
  TXeleFtdcFrontListType   FrontList;
  ///交易前置IP地址列表
  TXeleFtdcFrontIpListType FrontIpList;
  ///预留字段
  char Rsv[10];
};
static_assert(sizeof(CXeleFtdcRspExchangeFrontField) == 288, "CXeleFtdcRspExchangeFrontField size error");


///********以下是新增的结构体********///
/// 结构体说明: 柜台事件通知域结构体
/// 对齐说明      : 字段1字节对齐，结构体32字节对齐, 总共288字节;
/// 详细说明       : 
/// 1. EventType 事件类型，表示公有事件(通知所有投资者)/私有事件(只通知推送指定投资者);
/// 2. EventID 事件ID，表示具体的事件号;
/// 3. EventData: 事件数据， 不同的事件，数据的结构体不一样;事件长度也不一样；
struct CXeleFtdcNotifyEventField {
    ///通知事件日期(当前交易日)
    TXeleFtdcDateType    TradingDay;
    ///通知事件时间(系统时间)
    TXeleFtdcDateType    EventTime;
	///通知事件类型
	TXeleFtdcEventType   EventType;
	///通知事件ID
	TXeleFtdcEventIDType EventID;
    ///通知事件数据长度
	TXeleFtdcEventLenType EventLen;
    ///通知事件的数据
	TXeleFtdcEventDataType EventData;
    ///预留字段
    char        Rsv[2];
};
static_assert(sizeof(CXeleFtdcNotifyEventField) == 288, "CXeleFtdcNotifyEventField size error");


/// 结构体说明: 前置状态事件通知域结构体
/// 对齐说明      : 字段1字节对齐，结构体32字节对齐, 总共32字节;
/// 详细说明       : 
/// 1. 该结构体也即 CXeleFtdcNotifyEventField结构体中的EventData具体类型
/// 2. 前置状态事件当前只有 前置链接和断链的事件
/// 3. 该事件属于共有事件
struct CXeleFtdcEventFrontField {
    /// 前置编号
    TXeleFtdcFrontIDType    FrontID;
    /// 前置IP
    TXeleFtdcFrontIPType    FrontIP;    
    /// 交易所ID
    TXeleFtdcExchangeDescriptorType   ExchangeID;
    ///预留字段
    char        Rsv[11];
};
static_assert(sizeof(CXeleFtdcEventFrontField) == 32, "CXeleFtdcEventFrontField size error");

/// 结构体说明: 柜台系统特性信息域结构体
/// 对齐说明      : 字段1字节对齐，结构体32字节对齐, 总共256字节;
/// 详细说明      : 客户端登录时会推送给API;
struct CXeleFtdcSystemFeatureField
{
    ///交易所ID
    TXeleFtdcExchangeDescriptorType ExchangeID;
	///期权占用保证金价格类型
	TXeleFtdcOptionPriceType	    OptionPriceType;
    ///期权冻结保证金价格类型
    TXeleFtdcOptionFrozenPriceType  OptionFrozenPriceType;
    ///预留字段
    char                            Rsv[253];
};
static_assert(sizeof(CXeleFtdcSystemFeatureField) == 256, "CXeleFtdcSystemFeatureField size error");

/// 结构体说明: 投资者普通风控事件通知域结构体
/// 对齐说明      : 字段1字节对齐，结构体32字节对齐, 总共32字节;
/// 详细说明       : 
/// 1. 该结构体也即 CXeleFtdcNotifyEventField结构体中的EventData具体类型
/// 2. 投资者普通风控事件通知(当前只有投资者交易权限的事件)
/// 3. 该事件属于私有事件
struct CXeleFtdcEventInvestorPrcField {
    ///< 风控的具体ID类型 
    TXeleFtdcInvestorPrcIDType   PrcID;
    ///< 投资者名称
    TXeleFtdcInvestorIDType      InvestorID;
    ///<投资者风控值
    TXeleFtdcPrcValueType        PrcValue; 
    ///预留字段
    char       Rsv[4];
};
static_assert(sizeof(CXeleFtdcEventInvestorPrcField) == 32, "CXeleFtdcEventInvestorPrcField size error");


/// 结构体说明: 投资者合约风控事件通知域结构体
/// 对齐说明      : 字段1字节对齐，结构体64字节对齐, 总共64字节;
/// 详细说明       : 
/// 1. 该结构体也即 CXeleFtdcNotifyEventField结构体中的EventData具体类型
/// 2. 投资者合约相关风控事件通知(暂未使用)
/// 3. 该事件属于私有事件
struct CXeleFtdcEventInstrumentPrcField {
    ///< 风控的具体ID类型 
    TXeleFtdcInvestorPrcIDType   PrcID;
    ///< 投资者名称
    TXeleFtdcInvestorIDType      InvestorID;
    ///<合约名称
    TXeleFtdcInstrumentIDType    InstrumentID;
    ///<投资者风控值
    TXeleFtdcPrcValueType        PrcValue; 
    ///预留字段
    char       Rsv[5];
};
static_assert(sizeof(CXeleFtdcEventInstrumentPrcField) == 64, "CXeleFtdcEventInstrumentPrcField size error");


/// 结构体说明: 投资者日初资金域结构体
/// 对齐说明      : 字段1字节对齐，结构体64字节对齐, 总共64字节;
/// 详细说明       : 
/// 1. 包含的是该投资者日初资金
struct CXeleFtdcPreFundsField {
	///投资者账号
	TXeleFtdcInvestorIDType	  InvestorID;
	///子账号
	TXeleFtdcInvestorIDType	  SubInvestorID;    
    ///老仓保证金
    TXeleFtdcMoneyType        PreMargin;
    ///交割保证金
    TXeleFtdcMoneyType        DeliveryMargin;
    ///静态权益
    TXeleFtdcMoneyType        PreBalance;
    ///预留字段
    char     Rsv[32];
};
static_assert(sizeof(CXeleFtdcPreFundsField) == 96, "CXeleFtdcPreFundsField size error");


/// 结构体说明: 投资者日初持仓域结构体
/// 对齐说明      : 字段1字节对齐，结构体64字节对齐, 总共64字节;
/// 详细说明       : 
/// 1. 包含的是该投资者日初持仓(也即昨持仓)
struct CXeleFtdcPrePositionField {
    ///投资者账号
	TXeleFtdcInvestorIDType	 InvestorID;
 	///合约代码
	TXeleFtdcInstrumentIDType InstrumentID;
    ///投保标记
    TXeleFtdcHedgeFlagType   HedgeFlag;
    ///多头昨持仓信息
    TXeleFtdcPositionType    LongYdPosition;
    ///空头昨持仓信息
    TXeleFtdcPositionType    ShortYdPosition;
    ///预留字段
    char            Rsv[36];
};
static_assert(sizeof(CXeleFtdcPrePositionField) == 96, "CXeleFtdcPrePositionField size error");

/// 结构体说明: 单腿合约明细
/// 对齐说明  : 字段1字节对齐，总共96字节;
/// 详细说明       :
/// 1. 包含的是该投资者日初持仓(也即昨持仓)
struct CXeleFtdcPreSinglePositionField {
  /// 交易日
  TXeleFtdcTradingDayType       TradingDay;
  /// 交易所ID
  TXeleFtdcExchangeDescriptorType ExchangeID;
  /// 投资者账号
  TXeleFtdcInvestorIDType       InvestorID;
  /// 合约索引值
  TXeleFtdcInstrumentIndexType  InstrumentIndex;
  /// 合约代码
  TXeleFtdcInstrumentIDType     InstrumentID;
  /// 投保标记
  TXeleFtdcHedgeFlagType        HedgeFlag;
  /// 交易ID
  TXeleFtdcTradeIDType          TradeId;
  /// 买卖方向
  TXeleFtdcDirectionType        Diretion;
  /// 持仓数量
  TXeleFtdcVolumeType           Volume;
  /// 昨结算价
  TXeleFtdcPriceType            LastSettlementPrice;
  ///预留字段
  char            Rsv[36];
};
static_assert(sizeof(CXeleFtdcPreSinglePositionField) == 128, "CXeleFtdcPreSinglePositionField size error");


/// 结构体说明: 投资者基本保证金率/手续费率 域结构体
/// 对齐说明      : 字段1字节对齐，结构体32字节对齐, 总共160字节;
/// 详细说明       : 
/// 1. 包含的是该投资者合约保证金率/手续费率信息
struct CXeleFtdcPreInstrumentRatioField
{
    ///投资者账号
	TXeleFtdcInvestorIDType	  InvestorID;
 	///合约代码
	TXeleFtdcInstrumentIDType InstrumentID;
    ///品种信息
    TXeleFtdcProductIDType    ProductID;
    ///合约品种类型
    TXeleFtdcProductClassType ProductClass;
    ///投保标记
    TXeleFtdcHedgeFlagType    HedgeFlag; 

    /// 保证金率
    ///买方向保证金率按金额
    TXeleFtdcMoneyType        LongMrgMoney;
    ///买方向保证金率按数量
    TXeleFtdcMoneyType        LongMrgVolume;
    ///卖方向保证金率按金额
    TXeleFtdcMoneyType        ShortMrgMoney;
    ///卖方向保证金率按数量
    TXeleFtdcMoneyType        ShortMrgVolume;  
    ///是否相对交易所收取
    TXeleFtdcMarginRelativeType  IsRelative;

    /// 手续费率
    ///开仓手续费按金额
    TXeleFtdcMoneyType          OpenCommMoney;
    ///开仓手续费按数量
    TXeleFtdcMoneyType          OpenCommVolume;
    ///平仓手续费按金额
    TXeleFtdcMoneyType          CloseCommMoney;
    ///平仓手续费按数量
    TXeleFtdcMoneyType          CloseCommVolume;
    ///平今手续费率按金额
    TXeleFtdcMoneyType          CloseTodayCommMoney;
    ///平今手续费率按数量
    TXeleFtdcMoneyType          CloseTodayCommVolume;    
    ///预留字段
    char            Rsv[46];
};
static_assert(sizeof(CXeleFtdcPreInstrumentRatioField) == 192, "CXeleFtdcPreInstrumentRatioField size error");


/// 结构体说明: 市场合约的基本信息域结构体
/// 对齐说明      : 字段1字节对齐，结构体32字节对齐, 总共256字节;
/// 详细说明       : 
/// 1. 包含的是市场所有合约的基本信息
struct CXeleFtdcPreInstrumentField
{
    /// 交易所ID
    TXeleFtdcExchangeDescriptorType    ExchangeID;
    /// 合约序号
    TXeleFtdcInstrumentIndexType       InstrumentIndex;
 	///合约代码
	TXeleFtdcInstrumentIDType	       InstrumentID;
    ///品种组ID
    TXeleFtdcProductGroupIDType	       ProductGroupID;
    ///品种信息
    TXeleFtdcProductIDType             ProductID;
    ///产品类型
    TXeleFtdcProductClassType          ProductClass;  
    ///基础商品代码
    TXeleFtdcInstrumentIDType          UnderlyingInstrID;
    ///持仓类型
    TXeleFtdcPositionTypeType          PositionType;
    ///交割年份
    TXeleFtdcYearType                  DeliveryYear;
    ///交割月份
    TXeleFtdcMonthType                 DeliveryMonth;
    ///最大市价单手数
    TXeleFtdcVolumeType                MaxMarketOrderVolume;
    ///最小市价单手数
    TXeleFtdcVolumeType                MinMarketOrderVolume;
    ///最大限价单手数
    TXeleFtdcVolumeType                MaxLimitOrderVolume;
    ///最小限价单手数
    TXeleFtdcVolumeType                MinLimitOrderVolume;
    ///最小变动价位
    TXeleFtdcPriceType                 PriceTick;
    ///合约数量乘数
    TXeleFtdcMultipleVolumeType        VolumeMultiple;
    ///合约基础商品乘数
    TXeleFtdcUnderlyingMultipleType    UnderlyingMultiple;
    ///成交价(期权使用)
    TXeleFtdcPriceType                 StrikePrice;   
    ///'1':看涨, '2':看跌(期权使用)
    TXeleFtdcOptionsTypeType           OptionsType;
    ///是否支持单向大边,'1':支持 '0':不支持
    TXeleFtdcMaxMarginSideType         MaxMarginSideAlgorithm;
    ///当前是否交易
    TXeleFtdcBoolType                  IsTrading;
    ///创建日
    TXeleFtdcDateType                  CreateDate;
    ///上市日
    TXeleFtdcDateType                  OpenDate;
    ///到期日
    TXeleFtdcDateType                  ExpireDate;
    ///开始交割日
    TXeleFtdcDateType                  StartDelivDate;
    ///最后交割日
    TXeleFtdcDateType                  EndDelivDate;  
    ///昨结算价格
    TXeleFtdcPriceType                 PreSettlementPrice;   
    ///收盘价格
    TXeleFtdcPriceType                 PreClosePrice;
    ///涨停价
    TXeleFtdcPriceType                 UpperLimitPrice;
    ///跌停价
    TXeleFtdcPriceType                 LowerLimitPrice;    
    ///预留字段
    char     Rsv[34];    
};
static_assert(sizeof(CXeleFtdcPreInstrumentField) == 256, "CXeleFtdcPreInstrumentField size error");

/// 结构体说明: 组合合约基本信息域结构体
/// 对齐说明      : 字段1字节对齐，总共160字节;
/// 详细说明      :
struct CXeleFtdcCombInstrumentField
{
  /// 交易所ID
  TXeleFtdcExchangeDescriptorType     ExchangeID;
  /// 组合合约索引
  TXeleFtdcCombInstrumentIndexType    CombInstrumentIndex;
  /// 组合合约代码
  TXeleFtdcCombInstruIDType           CombInstrumentID;
  /// 组合合约投资套保标记
  TXeleFtdcCombHedgeType              CombHedgeFlag;
  /// 组合合约买卖方向
  TXeleFtdcCombDirectionType          CombDirection;
  /// 组合类型
  TXeleFtdcCombType                   CombinationType;
  /// 组合交易ID
  TXeleFtdcTradeIDType                TradeGroupId;
  /// 品种代码
  TXeleFtdcProductIDType              ProductId;

  ///组合合约-左腿合约索引
  TXeleFtdcInstrumentIndexType        LeftInstrumentIndex;
  ///组合合约-左腿合约ID
  TXeleFtdcInstrumentIDType           LeftInstrumentID;
  ///组合合约-左腿合约买卖方向
  TXeleFtdcDirectionType              LeftDirection;

  ///组合合约-右腿合约索引
  TXeleFtdcInstrumentIndexType        RightInstrumentIndex;
  ///组合合约-右腿合约ID
  TXeleFtdcInstrumentIDType           RightInstrumentID;
  ///组合合约-右腿合约买卖方向
  TXeleFtdcDirectionType              RightDirection;
  ///预留字段
  char                                Rsv[22];
};
static_assert(sizeof(CXeleFtdcCombInstrumentField) == 160, "CXeleFtdcCombInstrumentField size error");

/// 结构体说明: 该投资者的出入金操作信息域结构体
/// 对齐说明      : 字段1字节对齐，结构体32字节对齐, 总共96字节;
/// 详细说明       : 
/// 1. 包含的是该投资者的出入金操作信息
struct CXeleFtdcRtnSeqDepositField
{
	///流中唯一的序列号
	TXeleFtdcSequenceNoType       UniqSequenceNo;
	///出入金类型
	TXeleFtdcDepositIOType	      DepositType;
	///有效标志-有效或冲正
	TXeleFtdcAvailabilityFlagType DepositFlag;
	///出入金方向
	TXeleFtdcDepositDirectionType Direction;
	///交易日
	TXeleFtdcTradingDayType	      TradingDay;
	///流水号
	TXeleFtdcDepositSeqNoType     SequenceNo;
	///时间
	TXeleFtdcTimeType	          Time;
	///经纪公司代码
	TXeleFtdcBrokerIDType	      BrokerID;
	///投资者代码
	TXeleFtdcInvestorIDType       InvestorID;
	///出入金金额
	TXeleFtdcMoneyType        	  Amount;
	///币种代码
	TXeleFtdcCurrencyIDType	      CurrencyID;
	///是否已分配
	TXeleFtdcBoolType	          IsAssign;    
    ///预留字段
    char                          Rsv[41];
};
static_assert(sizeof(CXeleFtdcRtnSeqDepositField) == 128, "CXeleFtdcRtnSeqDepositField size error");

/// 结构体说明: 查询合约序号请求的信息域结构体
/// 对齐说明      : 字段1字节对齐，结构体64字节对齐, 总共64字节;
/// 详细说明      : 
/// 1. 如果填写了ProductID + InstrumentID; 表示查询指定品种和合约的index;
/// 2. 如果只填写ProductID， 表示查询该品种下的所有合约的index
/// 3. 如果都没有填写, 表示查询所有的品种所有的合约index
struct CXeleFtdcQryInstrumentIndexField {
    ///产品代码
    TXeleFtdcProductIDType    ProductID;
    ///合约代码
    TXeleFtdcInstrumentIDType InstrumentID;
    ///预留字段
    char                      Rsv[24]; 
};
static_assert(sizeof(CXeleFtdcQryInstrumentIndexField) == 64, "CXeleFtdcQryInstrumentIndexField size error");

/// 结构体说明: 查询合约序号的响应信息域结构体
/// 对齐说明      : 字段1字节对齐，结构体64字节对齐, 总共64字节;
/// 详细说明      : 
struct CXeleFtdcRspInstrumentIndexField {
    ///产品代码
    TXeleFtdcProductIDType       ProductID;
    ///合约代码
    TXeleFtdcInstrumentIDType    InstrumentID;
    ///合约序号
    TXeleFtdcInstrumentIndexType InstrumentIndex;
    ///预留字段
    char                         Rsv[20]; 
};
static_assert(sizeof(CXeleFtdcRspInstrumentIndexField) == 64, "CXeleFtdcRspInstrumentIndexField size error");

/// 结构体说明: 当进行压缩数据传输时，告诉接收端压缩后的扩展信息结构体
/// 对齐说明      : 字段1字节对齐，结构体64字节对齐, 总共128字节;
struct CXeleFtdcZipStreamHeaderField {
  /// 数据类型
  /*
  XTF_SEQ_ORDER_INFO = 7    //回报流水
  */
  uint32_t itemType;
  /// 原始数据的数据条目总个数
  uint32_t itemCount;
  /// 起始序号
  uint32_t beginSeqNo;
  /// 原始数据的总字节数
  unsigned long long rawDataSize ;
  /// 原始数据的MD5摘要值
  char rawDataMd5Sum[XELE_MD5_STRING_LEN_256+8];
  /// 压缩数据的总字节数
  unsigned long long zipDataSize ;
  /// 原始数据的MD5摘要值
  char zipDataMd5Sum[XELE_MD5_STRING_LEN_256+8];
  ///预留字段
  char                      Rsv[20];
};
static_assert(sizeof(CXeleFtdcZipStreamHeaderField) == 128, "CXeleFtdcZipStreamHeaderField size error");

#pragma pack(pop)

#ifdef __cplusplus
}
#endif

#endif
