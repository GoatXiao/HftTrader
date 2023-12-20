/**
 * @file        XTFErrorCode.h
 * @brief       错误码定义文件
 * @details     本文件定义API的常见错误码
 * @author      xinweiz
 * @date        2022-06-22
 * @version     v4.1
 * @copyright   南京艾科朗克信息科技有限公司
*/

#ifndef XTF_ERROR_CODE_H
#define XTF_ERROR_CODE_H

/// 错误码定义
const int ERRXTFAPI_NoError                       = 0; // 正确

const int ERRXTFAPI_InvalidParameter              = -10001; // 传入参数无效
const int ERRXTFAPI_NotSupported                  = -10002; // 方法暂不支持
const int ERRXTFAPI_NotFound                      = -10003; // 资源未找到
const int ERRXTFAPI_NoMemory                      = -10004; // 内存不足
const int ERRXTFAPI_TcpSendPartialData            = -12004; // TCP发送数据不完整

const int ERRXTFAPI_InvalidAccount                = -11001; // 账户不能为空
const int ERRXTFAPI_InvalidPassword               = -11002; // 密码不能为空
const int ERRXTFAPI_InvalidAppID                  = -11003; // 应用ID不能为空
const int ERRXTFAPI_InvalidAuthCode               = -11004; // 授权码不能为空
const int ERRXTFAPI_InvalidTradeIP                = -11005; // 交易服务IP地址不能为空
const int ERRXTFAPI_InvalidTradePort              = -11006; // 交易服务端口不能为0
const int ERRXTFAPI_InvalidQueryIP                = -11007; // 查询服务IP地址不能为空
const int ERRXTFAPI_InvalidQueryPort              = -11008; // 查询服务端口不能为0

const int ERRXTFAPI_TcpConnectionRefused          = -12311; // TCP连接被拒绝

const int ERRXTFAPI_FtdInvalidPacket              = -14001; // FTD数据报无效
const int ERRXTFAPI_FtdZeroLength                 = -14002; // FTD数据报长度为0
const int ERRXTFAPI_FtdBufferNoMemory             = -14003; // FTD数据报解析缓冲区空间不足
const int ERRXTFAPI_FtdBufferInvalid              = -14004; // FTD数据报缓冲区无效
const int ERRXTFAPI_FtdBufferTooLarge             = -14005; // FTD数据报缓冲区分配空间过大
const int ERRXTFAPI_FtdDecodeFailed               = -14101; // FTD数据报解析失败

const int ERRXTFAPI_UnzipBufferEmpty              = -15004; // 解压缓冲区不能为空
const int ERRXTFAPI_UnzipBufferTooLarge           = -15005; // 解压缓冲区分配空间过大
const int ERRXTFAPI_UnzipFailed                   = -15006; // 解压失败
const int ERRXTFAPI_UnzipInProgress               = -15007; // 正在处理流水数据解压

const int ERRXTFAPI_FunctionNotFinished           = -16001; // 同步函数正在执行中

const int ERRXTFAPI_InvalidImp                    = -20002; // 无效的会话实现
const int ERRXTFAPI_InvalidSpi                    = -20001; // 无效的SPI回调对象
const int ERRXTFAPI_NotStarted                    = -20004; // API未启动
const int ERRXTFAPI_NotLoggedIn                   = -20005; // API未登录
const int ERRXTFAPI_RepeatInit                    = -20006; // API重复初始化
const int ERRXTFAPI_RepeatStart                   = -20007; // API重复启动
const int ERRXTFAPI_RepeatStop                    = -20008; // API重复停止
const int ERRXTFAPI_RepeatLogin                   = -20009; // API重复登录
const int ERRXTFAPI_RepeatLogout                  = -20010; // API重复登出
const int ERRXTFAPI_WrongDataType                 = -20011; // 错误的数据类型
const int ERRXTFAPI_RetryMaxCount                 = -20014; // TCP重连已达最大次数
const int ERRXTFAPI_AccountChanged                = -20015; // 当前账户不能变更
const int ERRXTFAPI_CatchupFailed                 = -20016; // 数据流水追平失败
const int ERRXTFAPI_SequenceNotContinuous         = -20017; // 数据报序列号不连续
const int ERRXTFAPI_SessionNotFound               = -20020; // 没有找到会话

const int ERRXTFAPI_OrderInvalid                  = -21001; // 报单无效
const int ERRXTFAPI_OrderCreateFailed             = -21002; // 报单创建失败
const int ERRXTFAPI_OrderExisted                  = -21003; // 报单已存在
const int ERRXTFAPI_OrderNotFound                 = -21004; // 报单没有找到
const int ERRXTFAPI_OrderCountExceeded            = -21005; // 报单数量超过最大允许范围
const int ERRXTFAPI_OrderStatusNotAllowedCancel   = -21006; // 报单状态不允许撤单
const int ERRXTFAPI_OnlyGFDOrderAllowedCancel     = -21007; // 只允许撤销GFD报单
const int ERRXTFAPI_OrderFlagNotSupported         = -21008; // OrderFlag不支持当前设定值
const int ERRXTFAPI_OrderTypeNotSupported         = -21009; // OrderType不支持当前设定值
const int ERRXTFAPI_ExecOrderInvalid              = -21011; // 无效的行权/对冲报单
const int ERRXTFAPI_ExecOrderCreateFailed         = -21012; // 行权/对冲报单创建失败
const int ERRXTFAPI_ExecOrderExisted              = -21013; // 行权/对冲报单已存在
const int ERRXTFAPI_ExecOrderNotFound             = -21014; // 行权/对冲报单没有找到

const int ERRXTFAPI_InvalidHedgeFlag              = -23011; // 无效的HedgeFlag
const int ERRXTFAPI_InvalidInstrument             = -23001; // 无效的合约
const int ERRXTFAPI_InstrumentCreateFailed        = -23002; // 合约创建失败
const int ERRXTFAPI_InstrumentNotFound            = -23004; // 合约没有找到
const int ERRXTFAPI_InvalidProduct                = -24001; // 无效的品种
const int ERRXTFAPI_ProductCreateFailed           = -24002; // 品种创建失败
const int ERRXTFAPI_ProductNotFound               = -24004; // 品种没有找到
const int ERRXTFAPI_ProductGroupCreateFailed      = -25002; // 品种组创建失败
const int ERRXTFAPI_ProductGroupNotFound          = -25004; // 品种组没有找到
const int ERRXTFAPI_InvalidExchange               = -27001; // 无效的交易所
const int ERRXTFAPI_ExchangeNotFound              = -27004; // 交易所没有找到

const int ERRXTFAPI_ClosedByUser                  = 1001; // 用户主动关闭
const int ERRXTFAPI_ClosedByTimeout               = 1002; // 超时关闭
const int ERRXTFAPI_ClosedBySendError             = 1003; // 发送错误后关闭
const int ERRXTFAPI_ClosedByRecvError             = 1004; // 接收错误后关闭
const int ERRXTFAPI_ClosedByLoginError            = 1010; // 登录失败后关闭
const int ERRXTFAPI_ClosedByLogoutError           = 1011; // 登出失败后关闭
const int ERRXTFAPI_ClosedByZipStreamError        = 1012; // 流水数据解压处理错误后关闭

#endif
