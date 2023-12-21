#ifndef  __COMPILE_H
#define  __COMPILE_H

#define LOCAL_PORT 19998

#define __LATENCY_TEST

#define __SHFE
#define __CTP2MINI

//选择行情连接
//CTP2MINI行情
//#define __MD_CTP2MINI
//#define FEED_LOGGER_CTP
//#define FEED_STRATEGY_CTP

//EFVI行情
//#define __MD_EFVI
//#define FEED_LOGGER_EFVI
//#define FEED_STRATEGY_EFVI
//END 选择行情连接


//选择报盘连接
//#define __OFFER_CTP2MINI
//#define __OFFER_V10
//#define __OFFER_XELE
#define __OFFER_YD
//END 选择报盘连接

//盘后模拟 宏
//#define __SIMULATE
//#define FEED_LOGGER_SIMULATE
//#define FEED_STRATEGY_SIMULATE


#endif //__COMPILE_H