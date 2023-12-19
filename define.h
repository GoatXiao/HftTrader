#ifndef  __DEFINE_H__
#define  __DEFINE_H__

#include <cstdio>
#include <cstring>

#include <sched.h>
#include <pthread.h>
#include <sys/sysinfo.h>

#include <iostream>
#include <filesystem>
#include <fstream>
#include <atomic>

#include <map>
#include <set>

//编译选项头文件
#include "compile.h"

//第三方库
//#include "include/common/shmmap.h"
#include "include/common/Dispatcher.h"
#include "include/common/SPSCQueueOPT.h"
#include "include/common/SPSCVarQueueOPT.h"
//#include "include/common/SPSCQueue.h"
//#include "include/common/SPSCVarQueue.h"

#include "include/common/tsl/robin_map.h"
#include "include/common/StrHash.h"
#include "include/common/tscns.h"

enum CPUID
{
    // 可配置线程1-11
    CONSOLE_CPUID = 0,
    FEED_CTP_CPUID = 0,
    AGENT_CPUID = 12,
    FEED_SHFE_CPUID,
    FEED_DCE_CPUID,
    FEED_CZCE_CPUID,
    CALLBACK_CPUID,
    LOGGER_CPUID
};

enum THREAD_PRIORITY {
    THREAD_USER = 0,
    THREAD_AGENT = 0
};

enum QRY_INSTRUMENT_TYPE
{
    FUTURE_ONLY = 0,
    OPTION_ONLY,
    FUTURE_AND_OPTION
};

enum LOGGER_TYPE
{
    FEED_LOG = 0,
    COMPLETE_LOG,
    TRADE_LOG,
    SEND_LOG,
    CANCEL_LOG,
    ERR_LOG
};

enum CALLBACK_TYPE
{
    ORDER_CONFIRM = 0,
    ORDER_ERROR,
    ORDER_TRADE,
    ORDER_CANCEL,
    ORDER_CANCEL_ERR,
};

#define ORDERPOOL_NUM 1000000

enum PRODUTIONLINE_TYPE
{
    PLT_LOGGER = 0,
    PLT_FEED_SHFE,
    PLT_FEED_DCE,
    PLT_FEED_CZCE,
    PLT_FEED_SIMULATE,
    PLT_AGENT,
    PLT_USER
};

namespace fs = std::filesystem;

template <typename T> struct HashMap {
    using type = StrHash<INSTRUMENTLENGTH, T*, nullptr, 7>;
};

#endif //__DEFINE_H__
